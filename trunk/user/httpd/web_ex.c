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
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/reboot.h>
#include <net/if.h>
#include <linux/types.h>
#include <linux/sockios.h>

#ifndef __user
#define __user
#endif

#include <wireless.h>
#include <ralink_priv.h>
#include <notify_rc.h>
#include <rstats.h>
#include <bin_sem_asus.h>

#include "common.h"
#include "nvram_x.h"
#include "httpd.h"

#define GROUP_FLAG_REFRESH 	0
#define GROUP_FLAG_DELETE 	1
#define GROUP_FLAG_ADD 		2
#define GROUP_FLAG_REMOVE 	3

static int apply_cgi_group(webs_t wp, int sid, struct variable *var, const char *groupName, int flag);
static void nvram_clr_group_temp(struct variable *v);
static int nvram_generate_table(webs_t wp, char *serviceId, char *groupName);

static int nvram_modified = 0;
#if BOARD_HAS_5G_RADIO
static int wl_modified = 0;
#endif
static int rt_modified = 0;
static u64 restart_needed_bits = 0;

static char post_buf[32768] = {0};
static char next_host[128] = {0};
static char SystemCmd[128] = {0};
static int  group_del_map[MAX_GROUP_COUNT+2];

extern struct evDesc events_desc[];
extern int auth_nvram_changed;
#if defined (SUPPORT_HTTPS)
extern int http_is_ssl;
#endif

void
nvram_commit_safe(void)
{
	if (nvram_get_int("nvram_manual") == 1)
		return;

	nvram_commit();
}

void
sys_reboot(void)
{
#ifdef MTD_FLASH_32M_REBOOT_BUG
	doSystem("/sbin/mtd_storage.sh %s", "save");
	system("/bin/mtd_write -r unlock mtd1");
#else
	kill(1, SIGTERM);
#endif
}

char *
rfctime(const time_t *timep)
{
	static char s[201];
	struct tm tm;

	time_zone_x_mapping();
	setenv("TZ", nvram_safe_get("time_zone_x"), 1);
	memcpy(&tm, localtime(timep), sizeof(struct tm));
	strftime(s, 200, "%a, %d %b %Y %H:%M:%S %z", &tm);
	return s;
}

/******************************************************************************/
/*
 *	Redirect the user to another webs page
 */

static void
websRedirect(webs_t wp, const char *url)
{
	char *http_str;

#if defined (SUPPORT_HTTPS)
	if (http_is_ssl)
		http_str = "https";
	else
#endif
	http_str = "http";

	websWrite(wp, "<html><head>\r\n");
	if (strlen(next_host) > 0)
		websWrite(wp, "<meta http-equiv=\"refresh\" content=\"0; url=%s://%s/%s\">\r\n", http_str, next_host, url);
	else
		websWrite(wp, "<meta http-equiv=\"refresh\" content=\"0; url=%s\">\r\n", url);
	websWrite(wp, "<meta http-equiv=\"Content-Type\" content=\"text/html\">\r\n");
	websWrite(wp, "</head></html>\r\n");

	websDone(wp, 200);
}

static void
sys_script(char *name)
{
	char scmd[64];
	int u2ec_fifo;
#if defined(USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	snprintf(scmd, sizeof(scmd), "/tmp/%s", name);

	if (strcmp(name,"syscmd.sh")==0)
	{
		if (SystemCmd[0] && get_login_safe()) {
			char path_env[64];
			snprintf(path_env, sizeof(path_env), "PATH=%s", SYS_EXEC_PATH_OPT);
			putenv(path_env);
			doSystem("%s >/tmp/syscmd.log 2>&1\n", SystemCmd);
			SystemCmd[0] = '\0';
		} else {
			system("echo -n > /tmp/syscmd.log\n");
		}
	}
	else if (strcmp(name, "syslog.sh")==0)
	{
		;   // to nothing
	}
	else if (strcmp(name, "wan.sh")==0)
	{
		;
	}
	else if (strcmp(name, "printer.sh")==0)
	{
		;
	}
	else if (strcmp(name, "lpr_remove")==0)
	{
		kill_pidfile_s("/var/run/lpdparent.pid", SIGUSR2);
	}
	else if (!strcmp(name, "mfp_requeue")) 
	{
		fill_login_ip(s_addr, sizeof(s_addr));
		
		nvram_set_temp("mfp_ip_requeue", s_addr);
		u2ec_fifo = open("/var/u2ec_fifo", O_WRONLY|O_NONBLOCK);
		if (u2ec_fifo >= 0)
		{
			write(u2ec_fifo, "q", 1);
			close(u2ec_fifo);
		}
	}
	else if (!strcmp(name, "mfp_monopolize")) 
	{
		fill_login_ip(s_addr, sizeof(s_addr));
		
		nvram_set_temp("mfp_ip_monopoly", s_addr);
		u2ec_fifo = open("/var/u2ec_fifo", O_WRONLY|O_NONBLOCK);
		if (u2ec_fifo >= 0)
		{
			write(u2ec_fifo, "m", 1);
			close(u2ec_fifo);
		}
	}
	else if (strcmp(name, "wlan11a.sh")==0 || strcmp(name,"wlan11b.sh")==0)
	{
		;// do nothing
	}
	else if (strcmp(name, "eth_wan.sh")==0 || strcmp(name,"eth_lan1.sh")==0 || strcmp(name,"eth_lan2.sh")==0 ||
	                                          strcmp(name,"eth_lan3.sh")==0 || strcmp(name,"eth_lan4.sh")==0)
	{
		;// do nothing
	}
	else if (strcmp(name,"leases.sh")==0)
	{
		;// Nothing
	}
	else if (strcmp(name,"vpns_list.sh")==0)
	{
		;// Nothing
	}
	else if (strcmp(name,"iptable.sh")==0) 
	{
		;// TODO
	}
	else if (strcmp(name,"route.sh")==0)
	{
		;// TODO
	}
	else if (strcmp(name,"conntrack.sh")==0)
	{
		;// TODO
	}
	else if (strcmp(name,"ddnsclient")==0)
	{
		if (pids("inadyn"))
			doSystem("killall %s %s", "-SIGUSR1", "inadyn");
	}
	else if (strcmp(name,"hostname_check") == 0)
	{
		notify_rc("manual_ddns_hostname_check");
	}
	else if (strcmp(name,"dlna_rescan") == 0)
	{
		notify_rc("restart_dms_rescan");
	}
	else if (strstr(scmd, " ") == 0) // no parameter, run script with eval
	{
		eval(scmd);
	}
	else
		system(scmd);
}

static int
write_file_dos2unix(const char* value, const char* file_path)
{
	FILE *fp;
	int i_len;
	char *v1, *v2, *v3;
	char buf[MAX_FILE_LINE_SIZE];

	fp = fopen(file_path, "w");
	if (!fp)
		return -1;

	v1 = (char *)value;
	v3 = (char *)value + strlen(value);
	while ((v2 = strstr(v1, "\r\n"))) {
		i_len = v2 - v1;
		if (i_len > (MAX_FILE_LINE_SIZE - 2))
		    i_len = (MAX_FILE_LINE_SIZE - 2);
		strncpy(buf, v1, i_len);
		buf[i_len  ] = '\n';
		buf[i_len+1] = '\0';
		fputs(buf, fp);
		v1 = v2 + 2;
	}
	if (v1 < v3) {
		fputs(v1, fp);
		if (*(v3-1) != '\n')
			fputs("\n", fp);
	}

	fclose(fp);

	return 0;
}

static int
write_textarea_to_file(const char* value, const char* dir_name, const char* file_name)
{
	int file_type, ret;
	char *extensions;
	char temp_path[64];
	char real_path[128];

	if (!value)
		return 0;

	file_type = 0;
	extensions = strrchr(file_name, '.');
	if (extensions) {
		if (strcmp(extensions, ".key") == 0 ||
		    strcmp(extensions, ".crt") == 0 ||
		    strcmp(extensions, ".pem") == 0)
			file_type = 2; // this is key/cert
		else if (strcmp(extensions, ".sh") == 0)
			file_type = 1; // this is script
	} else {
		if (strcmp(file_name, "authorized_keys") == 0)
			file_type = 3; // this is ssh authorized_keys
	}

	snprintf(temp_path, sizeof(temp_path), "%s/.%s", "/tmp", file_name);
	snprintf(real_path, sizeof(real_path), "%s/%s", dir_name, file_name);

	if (file_type == 2) {
		if (strlen(value) < 3) {
			int ret = unlink(real_path);
			return (ret == 0) ? 1 : 0;
		}
		if (!strstr(value, "-----BEGIN ") || !strstr(value, "-----END "))
			return 0;
	} else if (file_type == 3) {
		if (strlen(value) < 7) {
			int ret = unlink(real_path);
			doSystem("rm -f %s/%s", "/home/admin/.ssh", "authorized_keys");
			return (ret == 0) ? 1 : 0;
		}
		if (!strstr(value, "ssh-") && !strstr(value, "ecdsa-"))
			return 0;
	}

	unlink(temp_path);
	if (write_file_dos2unix(value, temp_path) != 0)
		return 0;

	ret = 0;
	if (compare_text_files(real_path, temp_path) != 0) {
		if (write_file_dos2unix(value, real_path) == 0) {
			if (file_type == 3) {
				chmod(real_path, 0600);
				doSystem ("[ -f /home/admin/.ssh ] && rm /home/admin/.ssh");
				doSystem ("[ -d /home/admin/.ssh ] || mkdir -p -m 700 /home/admin/.ssh");
				doSystem("cp -f %s %s", real_path, "/home/admin/.ssh");
			}
			else if (file_type == 2)
				chmod(real_path, 0600);
			else if (file_type == 1)
				chmod(real_path, 0755);
			else
				chmod(real_path, 0644);
			ret = 1;
		}
	}

	unlink(temp_path);

	return ret;
}

static void
websScan(const char *query)
{
#define SCAN_MAX_VALUE_LEN 256
	unsigned int i, flag, i_len;
	const char *v1, *v2, *v3, *sp;
	char groupid[64];
	char value[SCAN_MAX_VALUE_LEN];

	i = 0;
	flag = 0;
	groupid[0] = 0;

	v1 = NULL;
	if (strlen(query) > 1)
		v1 = query;

	while (v1!=NULL) {
		v2 = strchr(v1+1, '=');
		v3 = strchr(v1+1, '&');
		if (!v2)
			break;
		
		if (v3!=NULL) {
			i_len = v3-v2-1;
			if (i_len > (SCAN_MAX_VALUE_LEN - 1))
				i_len = (SCAN_MAX_VALUE_LEN - 1);
			strncpy(value, v2+1, i_len);
			value[i_len] = 0;
		} else {
			snprintf(value, sizeof(value), "%s", v2+1);
		}
		
		if (v2 != NULL && ((sp = strchr(v1+1, ' ')) == NULL || (sp > v2))) {
			if (flag && strncmp(v1+1, groupid, strlen(groupid))==0) {
				if (i < MAX_GROUP_COUNT) {
					group_del_map[i] = atoi(value);
					if (group_del_map[i]==-1)
						break;
					i++;
				}
			} else if (strncmp(v1+1, "group_id", 8)==0) {
				snprintf(groupid, sizeof(groupid), "%s_s", value);
				flag = 1;
			}
		}
		v1 = strchr(v1+1, '&');
	}

	group_del_map[i] = -1;
}

static void
websApply(webs_t wp, const char *url)
{
	do_ej(url, wp);
	websDone(wp, 200);
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1
 * <% nvram_get_x("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get_x("undefined"); %> produces ""
 */
static int
ej_nvram_get_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name, *cn, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	cn = nvram_safe_get(name);
	for (c = cn; *c; c++) {
		if (*c < 0 || (*c >= 0x20 && *c != '"' && *c != '&' && *c != '<' && *c != '>'))
			ret += fprintf(wp, "%c", *c);
		else
			ret += fprintf(wp, "&#%d;", *c);
	}

	fflush(wp);

	return ret;
}

static int
ej_nvram_get_ddns(int eid, webs_t wp, int argc, char **argv)
{
	int ret = ej_nvram_get_x(eid, wp, argc, argv);

	if (!nvram_match("ddns_return_code", "ddns_query"))
		nvram_set_temp("ddns_return_code", "");

	return ret;
}


/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static int
ej_nvram_match_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name, *match, *output;

	if (ejArgs(argc, argv, "%s %s %s %s", &sid, &name, &match, &output) < 4) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	if (nvram_match(name, match))
	{
		return websWrite(wp, output);
	}

	return 0;
}

static int
ej_nvram_double_match_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name, *match, *output;
	char *sid2, *name2, *match2;

	if (ejArgs(argc, argv, "%s %s %s %s %s %s %s", &sid, &name, &match, &sid2, &name2, &match2, &output) < 7) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	if (nvram_match(name, match) && nvram_match(name2, match2))
	{
		return websWrite(wp, output);
	}

	return 0;
}

/*
 * Example: 
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static int
ej_nvram_match_both_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name, *match, *output, *output_not;

	if (ejArgs(argc, argv, "%s %s %s %s %s", &sid, &name, &match, &output, &output_not) < 5) 
	{
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	if (nvram_match(name, match))
	{
		return websWrite(wp, output);
	}
	else
	{
		return websWrite(wp, output_not);
	}
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1 192.168.39.248
 * <% nvram_get_list("lan_ipaddr", 0); %> produces "192.168.1.1"
 * <% nvram_get_list("lan_ipaddr", 1); %> produces "192.168.39.248"
 */
static int
ej_nvram_get_list_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name;
	int which;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s %d", &sid, &name, &which) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	ret += websWrite(wp, nvram_get_list_x(name, which));
	return ret;
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1 192.168.39.248
 * <% nvram_get_list("lan_ipaddr", 0); %> produces "192.168.1.1"
 * <% nvram_get_list("lan_ipaddr", 1); %> produces "192.168.39.248"
 */
static int
ej_nvram_get_buf_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name;
	int which;		

	if (ejArgs(argc, argv, "%s %s %d", &sid, &name, &which) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	
	return 0;
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1 192.168.39.248
 * <% nvram_get_table_x("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get_table_x("lan_ipaddr"); %> produces "192.168.39.248"
 */
static int
ej_nvram_get_table_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	ret += nvram_generate_table(wp, sid, name);
	return ret;
}

/*
 * Example: 
 * wan_proto=dhcp;dns
 * <% nvram_match_list("wan_proto", "dhcp", "selected", 0); %> produces "selected"
 * <% nvram_match_list("wan_proto", "static", "selected", 1); %> does not produce
 */
static int
ej_nvram_match_list_x(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name, *match, *output;
	int which;

	if (ejArgs(argc, argv, "%s %s %s %s %d", &sid, &name, &match, &output, &which) < 5) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (nvram_match_list_x(name, match, which))
		return websWrite(wp, output);

	return 0;
}

static int
ej_nvram_char_to_ascii(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, *name;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	char tmpstr[256];
	memset(tmpstr, 0x0, sizeof(tmpstr));
	char_to_ascii(tmpstr, nvram_safe_get(name));
	ret += websWrite(wp, "%s", tmpstr);

	return ret;
}

/* report system time */
static int
ej_uptime(int eid, webs_t wp, int argc, char **argv)
{
	time_t tm;

	time(&tm);
	return websWrite(wp, rfctime(&tm));
}

static int
dump_file(webs_t wp, char *filename)
{
	char *extensions;

	if (!f_exists(filename)) {
		return websWrite(wp, "%s", "");
	}

	extensions = strrchr(filename, '.');
	if (!get_login_safe() && extensions && strcmp(extensions, ".key") == 0) {
		return websWrite(wp, "%s", "# !!!This is hidden write-only secret key file!!!\n");
	}

	return do_f(filename, wp);
}

static int
ej_dump(int eid, webs_t wp, int argc, char **argv)
{
	char filename[128];
	char *file, *script;
	int ret;

	if (ejArgs(argc, argv, "%s %s", &file, &script) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	// run scrip first to update some status
	if (strlen(script) > 1)
		sys_script(script);

#if BOARD_HAS_5G_RADIO
	if (strcmp(file, "wlan11b.log")==0)
		return ej_wl_status_5g(eid, wp, 0, NULL);
	else
#endif
	if (strcmp(file, "wlan11b_2g.log")==0)
		return ej_wl_status_2g(eid, wp, 0, NULL);
	else if (strcmp(file, "leases.log")==0)
		return (ej_lan_leases(eid, wp, 0, NULL));
	else if (strcmp(file, "vpns_list.log")==0)
		return (ej_vpns_leases(eid, wp, 0, NULL));
	else if (strcmp(file, "iptable.log")==0)
		return (ej_nat_table(eid, wp, 0, NULL));
	else if (strcmp(file, "route.log")==0)
		return (ej_route_table(eid, wp, 0, NULL));
	else if (strcmp(file, "conntrack.log")==0)
		return (ej_conntrack_table(eid, wp, 0, NULL));

	ret = 0;

	if (strncmp(file, "httpssl.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_HTTPSSL_DIR, file+8);
	else if (strncmp(file, "ovpnsvr.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_OVPNSVR_DIR, file+8);
	else if (strncmp(file, "ovpncli.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_OVPNCLI_DIR, file+8);
	else if (strncmp(file, "dnsmasq.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_DNSMASQ_DIR, file+8);
	else if (strncmp(file, "scripts.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_SCRIPTS_DIR, file+8);
	else if (strncmp(file, "crontab.", 8)==0)
		snprintf(filename, sizeof(filename), "%s/%s", STORAGE_CRONTAB_DIR, nvram_safe_get("http_username"));
	else
		snprintf(filename, sizeof(filename), "%s/%s", "/tmp", file);

	ret+=dump_file(wp, filename);

	return ret;
}

static void
validate_cgi(webs_t wp, int sid)
{
	struct variable *v;
	char *value;
	char name[64];

	/* Validate and set variables in table order */
	for (v = GetVariables(sid); v->name != NULL; v++) {
		snprintf(name, sizeof(name), "%s", v->name);
		if ((value = websGetVar(wp, name, NULL))) {
			if (strcmp(v->longname, "Group") && strcmp(v->longname, "File"))
				nvram_set(v->name, value);
		}
	}
}

static char *
svc_pop_list(char *value, char key)
{
	char *v, *buf;

	if (!value || *value=='\0')
		return NULL;

	buf = value;
	v = strchr(buf, key);

	if (v!=NULL) {
		*v = '\0';
		return buf;
	}

	return NULL;
}

#define WIFI_IWPRIV_CHANGE_BIT	(1<<0)
#define WIFI_COMMON_CHANGE_BIT	(1<<1)
#define WIFI_RADIO_CONTROL_BIT	(1<<2)
#define WIFI_GUEST_CONTROL_BIT	(1<<3)
#define WIFI_SCHED_CONTROL_BIT	(1<<4)

static void
set_wifi_param_int(const char* ifname, char* param, char* value, int val_min, int val_max)
{
	int i_value = atoi(value);
	if (i_value < val_min) i_value = val_min;
	if (i_value > val_max) i_value = val_max;

	doSystem("iwpriv %s set %s=%d", ifname, param, i_value);
}

static void
set_wifi_mrate(const char* ifname, const char* value, int is_aband)
{
	int i_mphy;
	int i_mmcs = 0;

	i_mphy = calc_mcast_tx_mode(atoi(value), is_aband, &i_mmcs);

	doSystem("iwpriv %s set %s=%d", ifname, "McastPhyMode", i_mphy);
	doSystem("iwpriv %s set %s=%d", ifname, "McastMcs", i_mmcs);
}

static void
validate_nvram_lan_param(const char *nvram_name, in_addr_t lan_addr, in_addr_t lan_mask)
{
	struct in_addr ina;
	in_addr_t lan_snet, test_addr;

	test_addr = inet_addr_safe(nvram_safe_get(nvram_name));
	if (test_addr == INADDR_ANY)
		return;

	test_addr = ntohl(test_addr);
	if ((test_addr & lan_mask) == (lan_addr & lan_mask))
		return;

	lan_snet = ~lan_mask;

	test_addr &= lan_snet;
	if (test_addr < 1)
		test_addr = 1;
	else
	if (test_addr > (lan_snet-1))
		test_addr = (lan_snet-1);

	if (test_addr == (lan_addr & lan_snet)) {
		if (test_addr < (lan_snet-1))
			test_addr += 1;
		else
			test_addr -= 1;
	}

	ina.s_addr = htonl((lan_addr & lan_mask) | test_addr);
	nvram_set(nvram_name, inet_ntoa(ina));
}

static void
validate_nvram_lan_subnet(void)
{
	char nvram_ip[32];
	in_addr_t lan_addr, lan_mask;
	int i, i_max_items;

	lan_addr = ntohl(inet_addr(nvram_safe_get("lan_ipaddr")));
	lan_mask = ntohl(inet_addr(nvram_safe_get("lan_netmask")));

	/* validate dhcp start */
	validate_nvram_lan_param("dhcp_start", lan_addr, lan_mask);

	/* validate dhcp end */
	validate_nvram_lan_param("dhcp_end", lan_addr, lan_mask);

	/* validate dhcp static IP */
	i_max_items = nvram_safe_get_int("dhcp_staticnum_x", 0, 0, 64);
	for (i = 0; i < i_max_items; i++) {
		snprintf(nvram_ip, sizeof(nvram_ip), "dhcp_staticip_x%d", i);
		validate_nvram_lan_param(nvram_ip, lan_addr, lan_mask);
	}

	if (get_ap_mode())
		return;

	/* validate port forwards IP */
	i_max_items = nvram_safe_get_int("vts_num_x", 0, 0, 64);
	for (i = 0; i < i_max_items; i++) {
		snprintf(nvram_ip, sizeof(nvram_ip), "vts_ipaddr_x%d", i);
		validate_nvram_lan_param(nvram_ip, lan_addr, lan_mask);
	}

	/* validate DMZ IP */
	validate_nvram_lan_param("dmz_ip", lan_addr, lan_mask);
}

static int
validate_asp_apply(webs_t wp, int sid)
{
	u64 event_mask;
	int user_changed = 0;
	int pass_changed = 0;
	int lanip_changed = 0;
	struct variable *v;
	char *value;
	char name[64];
	char buff[160];

	/* Validate and set variables in table order */
	for (v = GetVariables(sid); v->name != NULL; ++v) {
		snprintf(name, sizeof(name), "%s", v->name);
		
		value = websGetVar(wp, name, NULL);
		if (!value)
			continue;
		
		if (!get_login_safe() && (v->event_mask & EVM_BLOCK_UNSAFE))
			continue;
		
		event_mask = v->event_mask & ~(EVM_BLOCK_UNSAFE);
		
		if (!strcmp(v->longname, "Group"))
			continue;
		
		if (!strcmp(v->longname, "File")) {
			const char *file_name = v->name+8;
			
			if (!strncmp(v->name, "dnsmasq.", 8)) {
				if (write_textarea_to_file(value, STORAGE_DNSMASQ_DIR, file_name))
					restart_needed_bits |= event_mask;
			} else if (!strncmp(v->name, "scripts.", 8)) {
				if (write_textarea_to_file(value, STORAGE_SCRIPTS_DIR, file_name))
					restart_needed_bits |= event_mask;
			} else if (!strncmp(v->name, "crontab.", 8)) {
				if (write_textarea_to_file(value, STORAGE_CRONTAB_DIR, nvram_safe_get("http_username")))
					restart_needed_bits |= event_mask;
			}
#if defined (SUPPORT_HTTPS)
			else if (!strncmp(v->name, "httpssl.", 8)) {
				if (write_textarea_to_file(value, STORAGE_HTTPSSL_DIR, file_name))
					restart_needed_bits |= event_mask;
			}
#endif
#if defined(APP_OPENVPN)
			else if (!strncmp(v->name, "ovpnsvr.", 8)) {
				if (write_textarea_to_file(value, STORAGE_OVPNSVR_DIR, file_name))
					restart_needed_bits |= event_mask;
			} else if (!strncmp(v->name, "ovpncli.", 8)) {
				if (write_textarea_to_file(value, STORAGE_OVPNCLI_DIR, file_name))
					restart_needed_bits |= event_mask;
			}
#endif
			continue;
		}
		
		/* check NVRAM value is changed */
		if (!strcmp(nvram_safe_get(name), value))
			continue;
		
		if (!strcmp(v->name, "http_username") || !strcmp(v->name, "http_passwd")) {
			if (strlen(value) == 0)
				continue;
		}
		
		if (!strcmp(v->name, "http_username")) {
			size_t buf_div = sizeof(buff)/2;
			snprintf(buff, buf_div, "%s/%s", STORAGE_CRONTAB_DIR, nvram_safe_get(v->name));
			snprintf(buff+buf_div, buf_div, "%s/%s", STORAGE_CRONTAB_DIR, value);
			rename(buff, buff+buf_div);
		}
		
		nvram_set(v->name, value);
		nvram_modified = 1;
		
		if (!strcmp(v->name, "http_username"))
			user_changed = 1;
		
		if (!strcmp(v->name, "http_passwd"))
			pass_changed = 1;
		
		if (!strcmp(v->name, "lan_ipaddr") || !strcmp(v->name, "lan_netmask"))
			lanip_changed = 1;
		
		/* update sw_mode before nvram_commit */
		if (!strcmp(v->name, "wan_nat_x")) {
			int wan_nat_x = atoi(value);
			if (nvram_get_int("sw_mode") != 3)
				nvram_set_int("sw_mode", (wan_nat_x) ? 1 : 4);
		}
		
#if BOARD_HAS_5G_RADIO
		if (!strncmp(v->name, "wl_", 3) && strcmp(v->name, "wl_ssid2"))
		{
			if (!strcmp(v->name, "wl_ssid"))
			{
				memset(buff, 0, sizeof(buff));
				char_to_ascii(buff, value);
				nvram_set("wl_ssid2", buff);
			}
			
			if (!strcmp(v->name, "wl_TxPower"))
			{
				const char *wifn = find_wlan_if_up(1);
				if (wifn)
					set_wifi_param_int(wifn, "TxPower", value, 0, 100);
				
				wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
			else if (!strcmp(v->name, "wl_greenap"))
			{
				set_wifi_param_int(IFNAME_5G_MAIN, "GreenAP", value, 0, 1);
				
				wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#if defined (USE_WID_5G) && (USE_WID_5G==7612)
			else if (!strcmp(v->name, "wl_VgaClamp"))
			{
				const char *wifn = find_wlan_if_up(1);
				if (wifn)
					set_wifi_param_int(wifn, "VgaClamp", value, 0, 4);
				
				wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#endif
#if defined (USE_IGMP_SNOOP)
			else if (!strcmp(v->name, "wl_IgmpSnEnable"))
			{
				set_wifi_param_int(IFNAME_5G_MAIN, "IgmpSnEnable", value, 0, 1);
				
				wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#else
			else if (!strcmp(v->name, "wl_IgmpSnEnable"))
			{
				int i_m2u = atoi(value);
				brport_set_m2u(IFNAME_5G_MAIN, i_m2u);
				brport_set_m2u(IFNAME_5G_GUEST, i_m2u);
			}
#endif
			else if (!strcmp(v->name, "wl_mrate"))
			{
				set_wifi_mrate(IFNAME_5G_MAIN, value, 1);
				
				wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
			else if (!strcmp(v->name, "wl_guest_enable") ||
			         !strcmp(v->name, "wl_guest_time_x") ||
			         !strcmp(v->name, "wl_guest_time2_x") ||
			         !strcmp(v->name, "wl_guest_date_x"))
			{
				wl_modified |= WIFI_GUEST_CONTROL_BIT;
			}
			else if (!strcmp(v->name, "wl_radio_time_x") ||
			         !strcmp(v->name, "wl_radio_time2_x") ||
			         !strcmp(v->name, "wl_radio_date_x"))
			{
				wl_modified |= WIFI_SCHED_CONTROL_BIT;
			}
			else
			{
				wl_modified |= WIFI_COMMON_CHANGE_BIT;
			}
		}
#endif
		if (!strncmp(v->name, "rt_", 3) && strcmp(v->name, "rt_ssid2"))
		{
			if (!strcmp(v->name, "rt_ssid"))
			{
				memset(buff, 0, sizeof(buff));
				char_to_ascii(buff, value);
				nvram_set("rt_ssid2", buff);
			}
			
			if (!strcmp(v->name, "rt_TxPower"))
			{
				const char *wifn = find_wlan_if_up(0);
				if (wifn)
					set_wifi_param_int(wifn, "TxPower", value, 0, 100);
				
				rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
			else if (!strcmp(v->name, "rt_greenap"))
			{
				set_wifi_param_int(IFNAME_2G_MAIN, "GreenAP", value, 0, 1);
				
				rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#if defined (USE_WID_2G) && (USE_WID_2G==7602 || USE_WID_2G==7612)
			else if (!strcmp(v->name, "rt_VgaClamp"))
			{
				const char *wifn = find_wlan_if_up(0);
				if (wifn)
					set_wifi_param_int(wifn, "VgaClamp", value, 0, 4);
				
				rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#endif
#if defined (USE_IGMP_SNOOP) || defined(USE_RT3352_MII)
			else if (!strcmp(v->name, "rt_IgmpSnEnable"))
			{
				set_wifi_param_int(IFNAME_2G_MAIN, "IgmpSnEnable", value, 0, 1);
				
				rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
#else
			else if (!strcmp(v->name, "rt_IgmpSnEnable"))
			{
				int i_m2u = atoi(value);
				brport_set_m2u(IFNAME_2G_MAIN, i_m2u);
				brport_set_m2u(IFNAME_2G_GUEST, i_m2u);
			}
#endif
			else if (!strcmp(v->name, "rt_mrate"))
			{
				set_wifi_mrate(IFNAME_2G_MAIN, value, 0);
				
				rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
			}
			else if (!strcmp(v->name, "rt_guest_enable") ||
			         !strcmp(v->name, "rt_guest_time_x") ||
			         !strcmp(v->name, "rt_guest_time2_x") ||
			         !strcmp(v->name, "rt_guest_date_x"))
			{
				rt_modified |= WIFI_GUEST_CONTROL_BIT;
			}
			else if (!strcmp(v->name, "rt_radio_time_x") ||
			         !strcmp(v->name, "rt_radio_time2_x") ||
			         !strcmp(v->name, "rt_radio_date_x"))
			{
				rt_modified |= WIFI_SCHED_CONTROL_BIT;
			}
#if defined(USE_RT3352_MII)
			else if (!strcmp(v->name, "rt_radio_x"))
			{
				rt_modified |= WIFI_RADIO_CONTROL_BIT;
			}
#endif
			else
			{
				rt_modified |= WIFI_COMMON_CHANGE_BIT;
			}
		}
		
		if (event_mask) {
			restart_needed_bits |= event_mask;
			dbG("debug restart_needed_bits: 0x%llx\n", restart_needed_bits);
		}
	}

	if (user_changed || pass_changed)
		auth_nvram_changed = 1;

	if (user_changed)
		recreate_passwd_unix(1);
	else if (pass_changed)
		change_passwd_unix(nvram_safe_get("http_username"), nvram_safe_get("http_passwd"));

	if (lanip_changed)
		validate_nvram_lan_subnet();

	return (nvram_modified || restart_needed_bits) ? 1 : 0;
}

#if 0
int
pinvalidate(char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return  -1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if (0 == (accum % 10))
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}
#endif

static int
update_variables_ex(int eid, webs_t wp, int argc, char **argv)
{
	int sid;
	char *action_mode;
	char *sid_list;
	char *script;
	char *serviceId;
	int result;

	restart_needed_bits = 0;

	// assign control variables
	action_mode = websGetVar(wp, "action_mode", "");
	script = websGetVar(wp, "action_script", "");
	sid_list = websGetVar(wp, "sid_list", "");

	while ((serviceId = svc_pop_list(sid_list, ';')) != NULL) {
		sid = 0;
		while (GetServiceId(sid) != NULL) {
			if (!strcmp(GetServiceId(sid), serviceId))
				break;
			sid++;
		}
		
		if (!strcmp(action_mode, "  Save  ") || !strcmp(action_mode, " Apply ")) {
			if (!validate_asp_apply(wp, sid))
				websWrite(wp, "<script>no_changes_and_no_committing();</script>\n");
			else
				websWrite(wp, "<script>done_committing();</script>\n");
		}
		else if (!strcmp(action_mode, "Update")) {
			validate_asp_apply(wp, sid);
			/* block restart signal (only nvram update + call script) */
			restart_needed_bits = 0;
		}
		else {
			char group_id[64];
			snprintf(group_id, sizeof(group_id), "%s", websGetVar(wp, "group_id", ""));
			
			if (strlen(action_mode) > 0) {
				if (!strcmp(action_mode, " Add ")) {
					result = apply_cgi_group(wp, sid, NULL, group_id, GROUP_FLAG_ADD);
					if (result == 1) {
						nvram_set_int_temp(group_id, 1);
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
				}
				else if (!strcmp(action_mode, " Del ")) {
					result = apply_cgi_group(wp, sid, NULL, group_id, GROUP_FLAG_REMOVE);
					if (result == 1) {
						nvram_set_int_temp(group_id, 1);
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
				}
				else if (!strcmp(action_mode, " Restart ")) {
					struct variable *v;
					
					for (v = GetVariables(sid); v->name != NULL; ++v) {
						if (!strcmp(v->name, group_id))
							break;
					}
					
					validate_asp_apply(wp, sid);	// for some nvram with this group
					
					if (v->name && nvram_get_int(group_id) > 0) {
						restart_needed_bits |= (v->event_mask & ~(EVM_BLOCK_UNSAFE));
						dbG("group restart_needed_bits: 0x%llx\n", restart_needed_bits);
#if BOARD_HAS_5G_RADIO
						if (!strcmp(group_id, "RBRList") || !strcmp(group_id, "ACLList"))
							wl_modified |= WIFI_COMMON_CHANGE_BIT;
#endif
						if (!strcmp(group_id, "rt_RBRList") || !strcmp(group_id, "rt_ACLList"))
							rt_modified |= WIFI_COMMON_CHANGE_BIT;
						
						nvram_modified = 1;
						nvram_set_int_temp(group_id, 0);
						nvram_clr_group_temp(v);
					}
					
					if (nvram_modified)
						websWrite(wp, "<script>done_committing();</script>\n");
					else
						websWrite(wp, "<script>no_changes_and_no_committing();</script>\n");
				}
				
				validate_cgi(wp, sid);	// for some nvram with this group group
			}
		}
		
		sid_list = sid_list+strlen(serviceId)+1;
	}

	if (strlen(script) > 0) {
		if (!strcmp(script, "networkmap_refresh")) {
			eval("restart_networkmap");
			websWrite(wp, "<script>restart_needed_time(%d);</script>\n", 1);
		}
		else if (!strcmp(script, "mfp_monopolize"))
		{
			sys_script(script);
			websWrite(wp, "<script>restart_needed_time(%d);</script>\n", 2);
			return 0;
		}
		else
		{
			sys_script(script);
		}
		
		websWrite(wp, "<script>done_committing();</script>\n");
		
		return 0;
	}

	if (restart_needed_bits != 0 && (!strcmp(action_mode, " Apply ") || !strcmp(action_mode, " Restart "))) {
		int i;
		u32 restart_total_time = 0;
		u32 max_time;
		
		i = 0;
		while (events_desc[i].notify_cmd) {
			if ((restart_needed_bits & events_desc[i].event_mask) != 0) {
				max_time = events_desc[i].max_time;
				
				if ((events_desc[i].event_mask & (EVM_RESTART_WAN|EVM_RESTART_IPV6)) != 0) {
					max_time = EVT_RESTART_WAN;
					if (nvram_match("wan_proto", "dhcp") || nvram_match("wan_proto", "static"))
						max_time = 3;
				}
				
				restart_total_time = MAX(restart_total_time, max_time);
			}
			i++;
		}
		
		if ((restart_needed_bits & EVM_RESTART_WIFI2) != 0) {
			max_time = 1;
			if ((rt_modified & WIFI_COMMON_CHANGE_BIT) && nvram_get_int("rt_radio_x"))
				max_time = EVT_RESTART_WIFI2;
			restart_total_time = MAX(restart_total_time, max_time);
		}
		
#if BOARD_HAS_5G_RADIO
		if ((restart_needed_bits & EVM_RESTART_WIFI5) != 0) {
			max_time = 1;
			if ((wl_modified & WIFI_COMMON_CHANGE_BIT) && nvram_get_int("wl_radio_x"))
				max_time = EVT_RESTART_WIFI5;
			restart_total_time = MAX(restart_total_time, max_time);
		}
#endif
		if ((restart_needed_bits & EVM_RESTART_REBOOT) != 0)
			restart_total_time = MAX(EVT_RESTART_REBOOT, restart_total_time);
		
		websWrite(wp, "<script>restart_needed_time(%u);</script>\n", restart_total_time);
	}

	return 0;
}

static int
asus_nvram_commit(int eid, webs_t wp, int argc, char **argv)
{
	if (nvram_modified) {
		nvram_modified = 0;
		nvram_commit_safe();
	}

	return 0;
}

static int
ej_notify_services(int eid, webs_t wp, int argc, char **argv)
{
	int i;

	if (!restart_needed_bits)
		return 0;

	if ((restart_needed_bits & EVM_RESTART_REBOOT) != 0) {
		restart_needed_bits = 0;
		if (nvram_get_int("nvram_manual") == 1)
			nvram_commit();
		sys_reboot();
		return 0;
	}

	dbG("debug restart_needed_bits before: 0x%llx\n", restart_needed_bits);

	i = 0;
	while (events_desc[i].notify_cmd) {
		if ((restart_needed_bits & events_desc[i].event_mask) != 0) {
			restart_needed_bits &= ~events_desc[i].event_mask;
			restart_needed_bits &= ~events_desc[i].event_unmask;
			notify_rc(events_desc[i].notify_cmd);
		}
		i++;
	}

	if ((restart_needed_bits & EVM_RESTART_WIFI2) != 0) {
		restart_needed_bits &= ~EVM_RESTART_WIFI2;
		if (rt_modified) {
			if (rt_modified & WIFI_COMMON_CHANGE_BIT)
				notify_rc(RCN_RESTART_WIFI2);
			else {
				if (rt_modified & WIFI_IWPRIV_CHANGE_BIT)
					notify_rc("control_wifi_config_rt");
				
				if (rt_modified & WIFI_RADIO_CONTROL_BIT)
					notify_rc("control_wifi_radio_rt");
				
				if (rt_modified & WIFI_GUEST_CONTROL_BIT)
					notify_rc("control_wifi_guest_rt");
				
				if (rt_modified & WIFI_SCHED_CONTROL_BIT)
					nvram_set_int_temp("reload_svc_rt", 1);
			}
			rt_modified = 0;
		}
	}

	if ((restart_needed_bits & EVM_RESTART_WIFI5) != 0) {
		restart_needed_bits &= ~EVM_RESTART_WIFI5;
#if BOARD_HAS_5G_RADIO
		if (wl_modified) {
			if (wl_modified & WIFI_COMMON_CHANGE_BIT)
				notify_rc(RCN_RESTART_WIFI5);
			else {
				if (wl_modified & WIFI_IWPRIV_CHANGE_BIT)
					notify_rc("control_wifi_config_wl");
				
				if (wl_modified & WIFI_RADIO_CONTROL_BIT)
					notify_rc("control_wifi_radio_wl");
				
				if (wl_modified & WIFI_GUEST_CONTROL_BIT)
					notify_rc("control_wifi_guest_wl");
				
				if (wl_modified & WIFI_SCHED_CONTROL_BIT)
					nvram_set_int_temp("reload_svc_wl", 1);
			}
			wl_modified = 0;
		}
#endif
	}

	dbG("debug restart_needed_bits after: 0x%llx\n", restart_needed_bits);

	restart_needed_bits = 0;

	return 0;
}

#define IF_STATE_EXIST		1
#define IF_STATE_UP		2
#define IF_STATE_HAS_ADDR	3

static int
get_if_state(const char *ifname, char addr4[INET_ADDRSTRLEN])
{
	struct in_addr addr4_in;
	int ifstate = 0;
	int iflags = get_interface_flags(ifname);

	if (iflags >= 0) {
		ifstate = IF_STATE_EXIST;
		if (iflags & IFF_UP) {
			ifstate = IF_STATE_UP;
			addr4_in.s_addr = get_interface_addr4(ifname);
			if (addr4_in.s_addr != INADDR_ANY) {
				ifstate = IF_STATE_HAS_ADDR;
				strcpy(addr4, inet_ntoa(addr4_in));
			}
		}
	}

	return ifstate;
}

/*
Internet connection status code:
0 - Connected
1 - No cable attached (Ethernet)
2 - No connection to AP (APCli)
3 - No connection to BTS (Modem) // not implemented yet
4 - Network interface not ready
5 - Network interface wait DHCP
6 - Wait PPP interface connection
7 - Inactive PPP state
8 - No Default Gateway
9 - Subnets conflict
*/

#define INET_STATE_CONNECTED		0
#define INET_STATE_NO_ETH_LINK		1
#define INET_STATE_NO_AP_LINK		2
#define INET_STATE_NO_BS_LINK		3
#define INET_STATE_NETIF_NOT_READY	4
#define INET_STATE_NETIF_WAIT_DHCP	5
#define INET_STATE_PPP_WAIT		6
#define INET_STATE_PPP_INACTIVE		7
#define INET_STATE_NO_DGW		8
#define INET_STATE_SUBNETS_CONFLICT	9

static int
wanlink_hook(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char wan_dns[512], wan_mac[18], etherlink[40], apclilink[32];
	char wan_desc[32], tmp[64], prefix[16];
	char addr4_wan[INET_ADDRSTRLEN], addr4_man[INET_ADDRSTRLEN];
	char *wan0_ip, *wanx_ip, *wan0_gw, *wanx_gw, *wan_ip6, *lan_ip6, *wan_ifname, *man_ifname;
	int unit, status_code, wan_proto, wan_ifstate, man_ifstate, need_eth_link, phy_failed, ppp_mode;
	uint64_t wan_bytes_rx, wan_bytes_tx;
	long wan_uptime, wan_dltime, wan_lease, now;
#if defined (USE_IPV6)
	int ipv6_type;
	char *wan6_ifname = NULL;
	char addr6_wan[INET6_ADDRSTRLEN], addr6_lan[INET6_ADDRSTRLEN];
#endif

	wan0_ip = "";
	wan0_gw = "";
	wanx_ip = "";
	wanx_gw = "";
	wan_ip6 = "";
	lan_ip6 = "";
	ppp_mode = 0;
	wan_uptime = 0;
	wan_dltime = 0;
	wan_bytes_rx = 0;
	wan_bytes_tx = 0;

	/* current unit */
	unit = 0;

	phy_failed = 0;
	wan_ifstate = 0;
	man_ifstate = 0;
	status_code = INET_STATE_CONNECTED;
	etherlink[0] = 0;
	apclilink[0] = 0;
	addr4_wan[0] = 0;
	addr4_man[0] = 0;
	wan_desc[0] = 0;

	need_eth_link = 0;

	now = uptime();

	if (!get_ap_mode()) {
		snprintf(prefix, sizeof(prefix), "wan%d_", unit);
		snprintf(wan_mac, sizeof(wan_mac), "%s", nvram_safe_get("wan_hwaddr"));
		
		wan_proto = get_wan_proto(unit);
		wan_lease = nvram_get_int(strcat_r(prefix, "lease", tmp));
		wan_dltime = nvram_get_int(strcat_r(prefix, "dltime", tmp));
		wan_uptime = nvram_get_int(strcat_r(prefix, "uptime", tmp));
		wan_ifname = nvram_safe_get(strcat_r(prefix, "ifname_t", tmp));
		man_ifname = get_man_ifname(unit);
		
#if defined(USE_USB_SUPPORT)
		if (get_usb_modem_wan(0)) {
			if (nvram_get_int("modem_prio") == 2)
				need_eth_link |= 1;
			if (nvram_get_int("modem_type") == 3) {
				if (is_usbnet_interface(wan_ifname)) {
					wan_ifstate = get_if_state(wan_ifname, addr4_wan);
					if (wan_ifstate > 0) {
						unsigned char mac[8];
						
						if (get_interface_hwaddr(wan_ifname, mac) == 0) {
							sprintf(wan_mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
						}
						
						wan_bytes_rx = get_ifstats_bytes_rx(wan_ifname);
						wan_bytes_tx = get_ifstats_bytes_tx(wan_ifname);
					}
				}
				strcpy(wan_desc, "USB Modem (NDIS/RNDIS)");
			} else {
				if (strncmp(wan_ifname, "ppp", 3) != 0)
					wan_ifname = IFNAME_RAS;
#if defined (USE_IPV6)
				if (nvram_get_int("ip6_wan_if") == 0)
					wan6_ifname = wan_ifname;
#endif
				wan_ifstate = get_if_state(wan_ifname, addr4_wan);
				if (wan_ifstate > 0) {
					wan_bytes_rx = get_ifstats_bytes_rx(wan_ifname);
					wan_bytes_tx = get_ifstats_bytes_tx(wan_ifname);
				}
				
				ppp_mode = 1;
				
				// Dual access with RAS Modem
				if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
				    wan_proto == IPV4_WAN_PROTO_PPTP ||
				    wan_proto == IPV4_WAN_PROTO_L2TP) {
					man_ifstate = get_if_state(man_ifname, addr4_man);
					need_eth_link |= 1;
				}
				strcpy(wan_desc, "USB Modem (RAS)");
			}
		} else
#endif
		{
			int wisp = 0;
			
			if (is_man_wisp(man_ifname)) {
				struct iwreq wrq;
				unsigned char mac[8];
				
				if (get_interface_hwaddr(man_ifname, mac) == 0) {
					sprintf(wan_mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
						mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				}
				
				if (get_apcli_peer_connected(man_ifname, &wrq)) {
					sprintf(apclilink, "BSSID: %02X:%02X:%02X:%02X:%02X:%02X", 
						(unsigned char)wrq.u.ap_addr.sa_data[0],
						(unsigned char)wrq.u.ap_addr.sa_data[1],
						(unsigned char)wrq.u.ap_addr.sa_data[2],
						(unsigned char)wrq.u.ap_addr.sa_data[3],
						(unsigned char)wrq.u.ap_addr.sa_data[4],
						(unsigned char)wrq.u.ap_addr.sa_data[5]);
						
#if 0
					{
						RT_802_11_MAC_ENTRY me;
						
						if (get_apcli_wds_entry(man_ifname, &me)) {
						}
					}
#endif
				} else {
					strcpy(apclilink, "---");
					phy_failed = 2; // STA not connected
				}
				
				wisp = 1;
			} else {
				need_eth_link |= 2;
			}
			
			if (wan_proto == IPV4_WAN_PROTO_PPPOE ||
			    wan_proto == IPV4_WAN_PROTO_PPTP ||
			    wan_proto == IPV4_WAN_PROTO_L2TP) {
				if (strncmp(wan_ifname, "ppp", 3) != 0)
					wan_ifname = IFNAME_PPP;
#if defined (USE_IPV6)
				if (nvram_get_int("ip6_wan_if") == 0)
					wan6_ifname = wan_ifname;
#endif
				wan_ifstate = get_if_state(wan_ifname, addr4_wan);
				man_ifstate = get_if_state(man_ifname, addr4_man);
				
				/* skip PPPoE traffic collect with HW_NAT enabled */
				if (wan_ifstate > 0 && (wisp || wan_proto != IPV4_WAN_PROTO_PPPOE || nvram_get_int("hw_nat_mode") == 2)) {
					wan_bytes_rx = get_ifstats_bytes_rx(wan_ifname);
					wan_bytes_tx = get_ifstats_bytes_tx(wan_ifname);
				}
				
				ppp_mode = (wan_proto == IPV4_WAN_PROTO_L2TP) ? 2 : 1;
			} else {
				wan_ifstate = get_if_state(man_ifname, addr4_wan);
				
				if (wan_ifstate > 0 && (wisp ||
#if !defined (USE_SINGLE_MAC)
								strcmp(man_ifname, IFNAME_MAC2) == 0 ||
#endif
								nvram_get_int("hw_nat_mode") == 2
							)
				    ) {
					wan_bytes_rx = get_ifstats_bytes_rx(man_ifname);
					wan_bytes_tx = get_ifstats_bytes_tx(man_ifname);
				}
			}
			
			if (wan_proto == IPV4_WAN_PROTO_PPPOE)
				strcpy(wan_desc, "PPPoE");
			else if (wan_proto == IPV4_WAN_PROTO_PPTP)
				strcpy(wan_desc, "PPTP");
			else if (wan_proto == IPV4_WAN_PROTO_L2TP)
				strcpy(wan_desc, "L2TP");
			else if (wan_proto == IPV4_WAN_PROTO_IPOE_DHCP)
				strcpy(wan_desc, "Automatic IP");
			else
				strcpy(wan_desc, "Static IP");
		}
		
		if (wan_ifstate == IF_STATE_HAS_ADDR) {
			wan0_ip = addr4_wan;
			wan0_gw = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
			if (!is_valid_ipv4(wan0_gw)) {
				status_code = INET_STATE_NO_DGW;
				wan0_gw = "---";
			}
			if (wan_uptime > 0) {
				wan_uptime = now - wan_uptime;
				if (wan_uptime < 0)
					wan_uptime = 0;
				
				if (wan_dltime > 0 && wan_lease > 0) {
					wan_dltime = wan_lease - (now - wan_dltime);
					/* always show after expired */
					if (wan_dltime < 1)
						wan_dltime = 1;
				}
			}
		} else {
			wan0_ip = "---";
			wan0_gw = "---";
			wan_uptime = 0;
			if (ppp_mode) {
				if (ppp_mode == 2) {
					char *l2tpd = "xl2tpd";
#if defined (APP_RPL2TP)
					if (nvram_match("wan_l2tpd", "1"))
						l2tpd = "l2tpd";
#endif
					status_code = (pids(l2tpd)) ? INET_STATE_PPP_WAIT : INET_STATE_PPP_INACTIVE;
				} else
					status_code = (pids("pppd")) ? INET_STATE_PPP_WAIT : INET_STATE_PPP_INACTIVE;
			} else {
				if (wan_ifstate == IF_STATE_UP) {
					int wan_err = nvram_get_int(strcat_r(prefix, "err", tmp));
					status_code = (wan_err == 1) ? INET_STATE_SUBNETS_CONFLICT : INET_STATE_NETIF_WAIT_DHCP;
				} else
					status_code = INET_STATE_NETIF_NOT_READY;
			}
		}
		
		if (need_eth_link) {
			int eth_link = fill_eth_port_status(nvram_get_int("wan_src_phy"), etherlink);
			if (!eth_link && (need_eth_link & 2))
				phy_failed = 1; // No Ethernet port link
		}
		
		if (phy_failed == 1)
			status_code = INET_STATE_NO_ETH_LINK;
		else if (phy_failed == 2)
			status_code = INET_STATE_NO_AP_LINK;
		else if (phy_failed == 3)
			status_code = INET_STATE_NO_BS_LINK;
		
		if (man_ifstate == IF_STATE_HAS_ADDR) {
			wanx_ip = addr4_man;
			wanx_gw = nvram_safe_get("wanx_gateway");
			if (!is_valid_ipv4(wanx_gw))
				wanx_gw = "---";
		}
		
		if (wan_uptime > 0 && (wan_bytes_rx || wan_bytes_tx)) {
			uint64_t init_bytes_rx, init_bytes_tx;
			
			init_bytes_rx = strtoull(nvram_safe_get(strcat_r(prefix, "bytes_rx", tmp)), NULL, 10);
			init_bytes_tx = strtoull(nvram_safe_get(strcat_r(prefix, "bytes_tx", tmp)), NULL, 10);
			
			/* support only 64 bit counters */
			if (wan_bytes_rx < init_bytes_rx)
				wan_bytes_rx = 0;
			else
				wan_bytes_rx -= init_bytes_rx;
			
			if (wan_bytes_tx < init_bytes_tx)
				wan_bytes_tx = 0;
			else
				wan_bytes_tx -= init_bytes_tx;
		} else {
			wan_bytes_rx = 0;
			wan_bytes_tx = 0;
		}
		
#if defined (USE_IPV6)
		ipv6_type = get_ipv6_type();
		if (ipv6_type != IPV6_DISABLED) {
			if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD) {
				wan6_ifname = IFNAME_SIT;
			} else {
				if (!wan6_ifname)
					wan6_ifname = man_ifname;
			}
			
			if (get_ifaddr6(wan6_ifname, 0, addr6_wan))
				wan_ip6 = addr6_wan;
			else
				wan_ip6 = "---";
			
			if (get_ifaddr6(IFNAME_BR, 0, addr6_lan))
				lan_ip6 = addr6_lan;
			else
				lan_ip6 = "---";
		}
#endif
	} else {
		snprintf(wan_mac, sizeof(wan_mac), "%s", nvram_safe_get("lan_hwaddr"));
		
		if (nvram_match("lan_proto_x", "1"))
			strcpy(wan_desc, "Automatic IP");
		else
			strcpy(wan_desc, "Static IP");
		
		wan_ifstate = get_if_state(IFNAME_BR, addr4_wan);
		
		if (wan_ifstate == IF_STATE_HAS_ADDR) {
			wan0_ip = addr4_wan;
			wan0_gw = nvram_safe_get("lan_gateway_t");
			if (!is_valid_ipv4(wan0_gw)) {
				wan0_gw = "---";
				status_code = INET_STATE_NO_DGW;
			}
		} else {
			wan0_ip = "---";
			wan0_gw = "---";
			status_code = (wan_ifstate == IF_STATE_UP) ? INET_STATE_NETIF_WAIT_DHCP : INET_STATE_NETIF_NOT_READY;
		}
	}

	wan_dns[0] = 0;
	fp = fopen("/etc/resolv.conf", "r");
	if (fp) {
		int is_first = 1;
		char line_buf[96], dns_item[80];
		while (fgets(line_buf, sizeof(line_buf), fp)) {
			if (sscanf(line_buf, "nameserver %79s\n", dns_item) < 1)
				continue;
			if (strlen(dns_item) < 7)
				continue;
			if (strcmp(dns_item, "127.0.0.1") != 0) {
				if (is_first)
					is_first = 0;
				else
					strcat(wan_dns, "<br/>");
				strcat(wan_dns, dns_item);
			}
		}
		
		fclose(fp);
	}

	if (strlen(wan_dns) < 7)
		strcpy(wan_dns, "---");

	websWrite(wp, "function wanlink_status() { return %d;}\n", status_code);
	websWrite(wp, "function wanlink_etherlink() { return '%s';}\n", etherlink);
	websWrite(wp, "function wanlink_apclilink() { return '%s';}\n", apclilink);
	websWrite(wp, "function wanlink_uptime() { return %ld;}\n", wan_uptime);
	websWrite(wp, "function wanlink_dltime() { return %ld;}\n", wan_dltime);
	websWrite(wp, "function wanlink_type() { return '%s';}\n", wan_desc);
	websWrite(wp, "function wanlink_ip4_wan() { return '%s';}\n", wan0_ip);
	websWrite(wp, "function wanlink_gw4_wan() { return '%s';}\n", wan0_gw);
	websWrite(wp, "function wanlink_ip4_man() { return '%s';}\n", wanx_ip);
	websWrite(wp, "function wanlink_gw4_man() { return '%s';}\n", wanx_gw);
	websWrite(wp, "function wanlink_ip6_wan() { return '%s';}\n", wan_ip6);
	websWrite(wp, "function wanlink_ip6_lan() { return '%s';}\n", lan_ip6);
	websWrite(wp, "function wanlink_dns() { return '%s';}\n", wan_dns);
	websWrite(wp, "function wanlink_mac() { return '%s';}\n", wan_mac);
	websWrite(wp, "function wanlink_bytes_rx() { return %llu;}\n", wan_bytes_rx);
	websWrite(wp, "function wanlink_bytes_tx() { return %llu;}\n", wan_bytes_tx);

	return 0;
}

static int
lanlink_hook(int eid, webs_t wp, int argc, char **argv)
{
	int i;
	char nvram_param[20], port_status[40] = {0};

	websWrite(wp, "function ether_link_mode(idx){\n");
	websWrite(wp, " if(idx==%d) return %d;\n", 0, nvram_get_int("ether_link_wan"));
	for (i = 0; i < BOARD_NUM_ETH_EPHY-1; i++) {
		snprintf(nvram_param, sizeof(nvram_param), "ether_link_lan%d", i+1);
		websWrite(wp, " if(idx==%d) return %d;\n", i+1, nvram_get_int(nvram_param));
	}
	websWrite(wp, " return %d;\n", 0);
	websWrite(wp, "}\n\n");

	websWrite(wp, "function ether_flow_mode(idx){\n");
	websWrite(wp, " if(idx==%d) return %d;\n", 0, nvram_get_int("ether_flow_wan"));
	for (i = 0; i < BOARD_NUM_ETH_EPHY-1; i++) {
		snprintf(nvram_param, sizeof(nvram_param), "ether_flow_lan%d", i+1);
		websWrite(wp, " if(idx==%d) return %d;\n", i+1, nvram_get_int(nvram_param));
	}
	websWrite(wp, " return %d;\n", 0);
	websWrite(wp, "}\n\n");

	websWrite(wp, "function ether_link_status(idx){\n");
	for (i = 0; i < BOARD_NUM_ETH_EPHY; i++) {
		port_status[0] = '\0';
		fill_eth_port_status(i, port_status);
		websWrite(wp, " if(idx==%d) return '%s';\n", i, port_status);
	}
	websWrite(wp, " return '%s';\n", "No link");
	websWrite(wp, "}\n");

	return 0;
}

static int
wan_action_hook(int eid, webs_t wp, int argc, char **argv)
{
	int unit, needed_seconds = 1;
	char *wan_action = websGetVar(wp, "wan_action", "");

	unit = 0;

	if (!strcmp(wan_action, "Connect")) {
		needed_seconds = 5;
		if (nvram_match("wan_proto", "dhcp"))
			needed_seconds = 3;
		else if (nvram_match("wan_proto", "static"))
			needed_seconds = 2;
		notify_rc("manual_wan_reconnect");
	}
	else if (!strcmp(wan_action, "Disconnect")) {
		needed_seconds = 3;
		if (nvram_match("wan_proto", "static"))
			needed_seconds = 2;
		notify_rc("manual_wan_disconnect");
	}
	else if (!strcmp(wan_action, "WispReassoc")) {
		notify_rc("manual_wisp_reassoc");
	}
#if defined(USE_USB_SUPPORT)
	else if (!strcmp(wan_action, "ModemPrio")) {
		int modem_prio = atoi(websGetVar(wp, "modem_prio", ""));
		if (modem_prio >= 0 && modem_prio < 3 && nvram_get_int("modem_prio") != modem_prio) {
			int modem_used;
			int need_restart_wan = 0;
			
			nvram_set_int("modem_prio", modem_prio);
			nvram_commit_safe();
			
			modem_used = (get_usb_modem_wan(unit)) ? 1 : 0;
			if (modem_prio < 2) {
				need_restart_wan = (modem_used != modem_prio);
			} else if (modem_prio == 2) {
				if (!is_man_wisp(get_man_ifname(unit)))
					need_restart_wan = (modem_used == get_wan_ether_link_cached());
			}
			
			if (need_restart_wan) {
				notify_rc("auto_wan_reconnect");
				needed_seconds = 3;
			}
		}
	}
#endif

	websWrite(wp, "<script>restart_needed_time(%d);</script>\n", needed_seconds);
	return 0;
}

#if defined (APP_SCUT)
static int scutclient_action_hook(int eid, webs_t wp, int argc, char **argv)
{
	int needed_seconds = 2;
	char *scut_action = websGetVar(wp, "connect_action", "");

	if (!strcmp(scut_action, "Reconnect")) {
		notify_rc(RCN_RESTART_SCUT);
	}
	else if (!strcmp(scut_action, "Disconnect")) {
		notify_rc("stop_scutclient");
	}

	websWrite(wp, "<script>restart_needed_time(%d);</script>\n", needed_seconds);
	return 0;
}

static int scutclient_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	int status_code = pids("bin_scutclient");
	websWrite(wp, "function scutclient_status() { return %d;}\n", status_code);
	return 0;
}

static int scutclient_version_hook(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fstream = NULL;
	char ver[8];
	memset(ver, 0, sizeof(ver));
	fstream = popen("/usr/bin/bin_scutclient -V","r");
	if(fstream) {
		fgets(ver, sizeof(ver), fstream);
		pclose(fstream);
		if (strlen(ver) > 0)
			ver[strlen(ver) - 1] = 0;
		if (!(ver[0]>='0' && ver[0]<='9'))
			sprintf(ver, "%s", "unknown");
	} else {
		sprintf(ver, "%s", "unknown");
	}
	websWrite(wp, "function scutclient_version() { return '%s';}\n", ver);
	return 0;
}
#endif

#if defined (APP_MENTOHUST)
static int mentohust_action_hook(int eid, webs_t wp, int argc, char **argv)
{
	int needed_seconds = 2;
	char *action = websGetVar(wp, "connect_action", "");

	if (!strcmp(action, "Reconnect")) {
		notify_rc(RCN_RESTART_MENTOHUST);
	}
	else if (!strcmp(action, "Disconnect")) {
		notify_rc("stop_mentohust");
	}

	websWrite(wp, "<script>restart_needed_time(%d);</script>\n", needed_seconds);
	return 0;
}

static int mentohust_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	int status_code = pids("bin_mentohust");
	websWrite(wp, "function mentohust_status() { return %d;}\n", status_code);
	return 0;
}
#endif

#if defined (APP_SHADOWSOCKS)
static int shadowsocks_action_hook(int eid, webs_t wp, int argc, char **argv)
{
	int needed_seconds = 3;
	char *ss_action = websGetVar(wp, "connect_action", "");

	if (!strcmp(ss_action, "Reconnect")) {
		notify_rc(RCN_RESTART_SHADOWSOCKS);
	} else if (!strcmp(ss_action, "Update_chnroute")) {
		notify_rc(RCN_RESTART_CHNROUTE_UPD);
		needed_seconds = 1;
	} else if (!strcmp(ss_action, "Reconnect_ss_tunnel")) {
		notify_rc(RCN_RESTART_SS_TUNNEL);
	} else if (!strcmp(ss_action, "Update_gfwlist")) {
		notify_rc(RCN_RESTART_GFWLIST_UPD);
	}
	websWrite(wp, "<script>restart_needed_time(%d);</script>\n", needed_seconds);
	return 0;
}

static int shadowsocks_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	int ss_status_code = pids("ss-redir");
	websWrite(wp, "function shadowsocks_status() { return %d;}\n", ss_status_code);
	int ss_tunnel_status_code = pids("ss-local");
	websWrite(wp, "function shadowsocks_tunnel_status() { return %d;}\n", ss_tunnel_status_code);
	return 0;
}

static int rules_count_hook(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fstream = NULL;
	char count[8];
	memset(count, 0, sizeof(count));
	fstream = popen("cat /etc/storage/chinadns/chnroute.txt |wc -l","r");
	if(fstream) {
		fgets(count, sizeof(count), fstream);
		pclose(fstream);
	} else {
		sprintf(count, "%d", 0);
	}
	if (strlen(count) > 0)
		count[strlen(count) - 1] = 0;
	websWrite(wp, "function chnroute_count() { return '%s';}\n", count);
#if defined(APP_SHADOWSOCKS)
	memset(count, 0, sizeof(count));
	fstream = popen("grep ^server /etc/storage/gfwlist/dnsmasq_gfwlist.conf |wc -l","r");
	if(fstream) {
		fgets(count, sizeof(count), fstream);
		pclose(fstream);
	} else {
		sprintf(count, "%d", 0);
	}
	if (strlen(count) > 0)
		count[strlen(count) - 1] = 0;
	websWrite(wp, "function gfwlist_count() { return '%s';}\n", count);	
#endif
	return 0;
}

#endif

#if defined(APP_DNSFORWARDER)
static int dnsforwarder_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	int status_code = pids("dns-forwarder");
	websWrite(wp, "function dnsforwarder_status() { return %d;}\n", status_code);
	return 0;
}
#endif

static int
ej_detect_internet_hook(int eid, webs_t wp, int argc, char **argv)
{
	kill_pidfile_s("/var/run/detect_internet.pid", SIGHUP);

	return 0;
}

static int
wol_action_hook(int eid, webs_t wp, int argc, char **argv) 
{
	int i, sys_result;
	char *dst_mac, *p1, *p2, *pd;
	char wol_mac[18];

	dst_mac = websGetVar(wp, "dstmac", "");
	if (!dst_mac)
		return -1;

	if (strlen(dst_mac) == 12)
	{
		p1 = dst_mac;
		p2 = wol_mac;
		pd = ":";
		for (i=0; i < 6; i++) {
			memcpy(p2, p1, 2);
			if (i<5)
				memcpy(p2+2, pd, 1);
			p2+=3;
			p1+=2;
		}
		wol_mac[17] = '\0';
	}
	else if (strlen(dst_mac) == 17)
	{
		strcpy(wol_mac, dst_mac);
	}
	else
	{
		wol_mac[0] = '\0';
	}
	
	sys_result = -1;
	
	if (wol_mac[0])
		sys_result = doSystem("/usr/sbin/ether-wake -b -i %s %s", IFNAME_BR, wol_mac);
	
	if (sys_result == 0) 
	{
		nvram_set_temp("wol_mac_last", wol_mac);
		websWrite(wp, "{\"wol_mac\": \"%s\"}", wol_mac);
	}
	else
		websWrite(wp, "{\"wol_mac\": \"null\"}");
	
	return 0;
}

static int
nf_values_hook(int eid, webs_t wp, int argc, char **argv) 
{
	FILE *fp;
	char nf_count[32];
	
	fp = fopen("/proc/sys/net/netfilter/nf_conntrack_count", "r");
	if (fp) {
		nf_count[0] = 0;
		fgets(nf_count, 32, fp);
		fclose(fp);
		if (strlen(nf_count) > 0)
			nf_count[strlen(nf_count) - 1] = 0; /* get rid of '\n' */
	}
	else {
		sprintf(nf_count, "%d", 0);
	}
	
	websWrite(wp, "function nf_conntrack_count() { return '%s';}\n", nf_count);
	
	return 0;
}

static int
ej_firmware_caps_hook(int eid, webs_t wp, int argc, char **argv) 
{
#if defined(UTL_HDPARM)
	int found_utl_hdparm = 1;
#else
	int found_utl_hdparm = 0;
#endif
#if defined(APP_OPENVPN)
	int found_app_ovpn = 1;
#else
	int found_app_ovpn = 0;
#endif
#if defined(APP_MINIDLNA)
	int found_app_dlna = 1;
#else
	int found_app_dlna = 0;
#endif
#if defined(APP_FIREFLY)
	int found_app_ffly = 1;
#else
	int found_app_ffly = 0;
#endif
#if defined(APP_TRMD)
	int found_app_trmd = 1;
#else
	int found_app_trmd = 0;
#endif
#if defined(APP_ARIA)
	int found_app_aria = 1;
#else
	int found_app_aria = 0;
#endif
#if defined(APP_NFSD)
	int found_app_nfsd = 1;
#else
	int found_app_nfsd = 0;
#endif
#if defined(APP_SMBD)
	int found_app_smbd = 1;
#else
	int found_app_smbd = 0;
#endif
#if defined(APP_SMBD) || defined(APP_NMBD)
	int found_app_nmbd = 1;
#else
	int found_app_nmbd = 0;
#endif
#if defined(APP_FTPD)
	int found_app_ftpd = 1;
#else
	int found_app_ftpd = 0;
#endif
#if defined(APP_RPL2TP)
	int found_app_l2tp = 1;
#else
	int found_app_l2tp = 0;
#endif
#if defined(SRV_U2EC)
	int found_srv_u2ec = 1;
#else
	int found_srv_u2ec = 0;
#endif
#if defined(SRV_LPRD)
	int found_srv_lprd = 1;
#else
	int found_srv_lprd = 0;
#endif
#if defined(APP_SSHD)
	int found_app_sshd = 1;
#else
	int found_app_sshd = 0;
#endif
#if defined(APP_SCUT)
	int found_app_scutclient = 1;
#else
	int found_app_scutclient = 0;
#endif
#if defined(APP_MENTOHUST)
	int found_app_mentohust = 1;
#else
	int found_app_mentohust = 0;
#endif
#if defined(APP_TTYD)
	int found_app_ttyd = 1;
#else
	int found_app_ttyd = 0;
#endif
#if defined(APP_VLMCSD)
	int found_app_vlmcsd = 1;
#else
	int found_app_vlmcsd = 0;
#endif
#if defined(APP_NAPT66)
	int found_app_napt66 = 1;
#else
	int found_app_napt66 = 0;
#endif
#if defined(APP_SHADOWSOCKS)
	int found_app_shadowsocks = 1;
#else
	int found_app_shadowsocks = 0;
#endif
#if defined(APP_DNSFORWARDER)
	int found_app_dnsforwarder = 1;
#else
	int found_app_dnsforwarder = 0;
#endif
#if defined(APP_XUPNPD)
	int found_app_xupnpd = 1;
#else
	int found_app_xupnpd = 0;
#endif
#if defined(USE_IPV6)
	int has_ipv6 = 1;
#else
	int has_ipv6 = 0;
#endif

#if defined(USE_HW_NAT)
	int has_ipv4_ppe = 1;
#if defined(USE_IPV6_HW_NAT)
#if defined(USE_HW_NAT_V2)
	int has_ipv6_ppe = 2;
#else
	int has_ipv6_ppe = 1;
#endif
#else
	int has_ipv6_ppe = 0;
#endif
#else
	int has_ipv4_ppe = 0;
	int has_ipv6_ppe = 0;
#endif

#if (BOARD_RAM_SIZE < 64)
	int max_conn = 16384;
#elif (BOARD_RAM_SIZE < 128)
	int max_conn = 65536;
#elif (BOARD_RAM_SIZE < 256)
	int max_conn = 262144;
#else
	int max_conn = 327680;
#endif
#if defined (USE_NAND_FLASH)
	int has_mtd_rwfs = 1;
#else
	int has_mtd_rwfs = 0;
#endif
#if defined (USE_USB_SUPPORT)
	int has_usb = 1;
#if (BOARD_NUM_UPHY_USB3 > 0)
	int has_usb3 = 1;
#else
	int has_usb3 = 0;
#endif
#else
	int has_usb = 0;
	int has_usb3 = 0;
#endif
#if defined (USE_STORAGE)
	int has_stor = 1;
#else
	int has_stor = 0;
#endif
#if defined (SUPPORT_PEAP_SSL)
	int has_peap_ssl = 1;
#else
	int has_peap_ssl = 0;
#endif
#if defined (SUPPORT_HTTPS)
	int has_http_ssl = 1;
#else
	int has_http_ssl = 0;
#endif
#if defined (SUPPORT_DDNS_SSL)
	int has_ddns_ssl = 1;
#else
	int has_ddns_ssl = 0;
#endif
#if defined (USE_RT3352_MII)
	int has_inic_mii = 1;
#else
	int has_inic_mii = 0;
#endif
#if defined (USE_RTL8367)
	int has_switch_type = 0; // Realtek RTL8367
#elif defined (USE_MTK_GSW)
	int has_switch_type = 5; // MT7621/MT7623 Internal GSW (or External MT7530)
#elif defined (USE_MTK_ESW)
#if defined (CONFIG_RALINK_MT7620)
	int has_switch_type = 12; // MT7620 Embedded ESW
#elif defined (CONFIG_RALINK_MT7628)
	int has_switch_type = 11; // MT7628 Embedded ESW
#else
	int has_switch_type = 10; // RT3052/RT3352/RT5350 Embedded ESW
#endif
#endif
#if defined (BOARD_GPIO_BTN_ROUTER) || defined (BOARD_GPIO_BTN_AP)
	int has_btn_mode = 1;
#else
	int has_btn_mode = 0;
#endif
#if defined (USE_WID_5G) && (USE_WID_5G==7610 || USE_WID_5G==7612 || USE_WID_5G==7615) && BOARD_HAS_5G_11AC
	int has_5g_vht = 1;
#else
	int has_5g_vht = 0;
#endif
#if defined (USE_WID_5G) && USE_WID_5G==7615 && BOARD_HAS_5G_11AC
	int has_5g_mumimo = 1;
	int has_5g_txbf = 1;
	int has_5g_band_steering = 1;
#if defined (BOARD_MT7615_DBDC)
	int has_5g_160mhz = 0;
#else
	int has_5g_160mhz = 1;
#endif
#else
	int has_5g_mumimo = 0;
	int has_5g_txbf = 0;
	int has_5g_band_steering = 0;
	int has_5g_160mhz = 0;
#endif
#if defined (USE_WID_2G) && USE_WID_2G==7615
	int has_2g_turbo_qam = 1;
	int has_2g_airtimefairness = 1;
#else
	int has_2g_turbo_qam = 0;
	int has_2g_airtimefairness = 0;
#endif
#if defined (USE_WID_2G)
	int wid_2g = USE_WID_2G;
#else
	int wid_2g = 0;
#endif
#if defined (USE_WID_5G)
	int wid_5g = USE_WID_5G;
#else
	int wid_5g = 0;
#endif
#if defined (USE_SFE)
	int has_sfe = 1;
#else
	int has_sfe = 0;
#endif
#if defined (BOARD_MT7615_DBDC)
	int has_lan_ap_isolate = 0;
#else
	int has_lan_ap_isolate = 1;
#endif

	websWrite(wp,
		"function found_utl_hdparm() { return %d;}\n"
		"function found_app_ovpn() { return %d;}\n"
		"function found_app_dlna() { return %d;}\n"
		"function found_app_ffly() { return %d;}\n"
		"function found_app_torr() { return %d;}\n"
		"function found_app_aria() { return %d;}\n"
		"function found_app_nfsd() { return %d;}\n"
		"function found_app_smbd() { return %d;}\n"
		"function found_app_nmbd() { return %d;}\n"
		"function found_app_ftpd() { return %d;}\n"
		"function found_app_l2tp() { return %d;}\n"
		"function found_srv_u2ec() { return %d;}\n"
		"function found_srv_lprd() { return %d;}\n"
		"function found_app_sshd() { return %d;}\n"
		"function found_app_scutclient() { return %d;}\n"
		"function found_app_ttyd() { return %d;}\n"
		"function found_app_vlmcsd() { return %d;}\n"
		"function found_app_napt66() { return %d;}\n"
		"function found_app_dnsforwarder() { return %d;}\n"
		"function found_app_shadowsocks() { return %d;}\n"
		"function found_app_xupnpd() { return %d;}\n"
		"function found_app_mentohust() { return %d;}\n",
		found_utl_hdparm,
		found_app_ovpn,
		found_app_dlna,
		found_app_ffly,
		found_app_trmd,
		found_app_aria,
		found_app_nfsd,
		found_app_smbd,
		found_app_nmbd,
		found_app_ftpd,
		found_app_l2tp,
		found_srv_u2ec,
		found_srv_lprd,
		found_app_sshd,
		found_app_scutclient,
		found_app_ttyd,
		found_app_vlmcsd,
		found_app_napt66,
		found_app_dnsforwarder,
		found_app_shadowsocks,
		found_app_xupnpd,
		found_app_mentohust
	);

	websWrite(wp,
		"function support_ipv6() { return %d;}\n"
		"function support_ipv6_ppe() { return %d;}\n"
		"function support_ipv4_ppe() { return %d;}\n"
		"function support_peap_ssl() { return %d;}\n"
		"function support_http_ssl() { return %d;}\n"
		"function support_ddns_ssl() { return %d;}\n"
		"function support_min_vlan() { return %d;}\n"
		"function support_max_conn() { return %d;}\n"
		"function support_mtd_rwfs() { return %d;}\n"
		"function support_btn_mode() { return %d;}\n"
		"function support_usb() { return %d;}\n"
		"function support_usb3() { return %d;}\n"
		"function support_num_usb() { return %d;}\n"
		"function support_storage() { return %d;}\n"
		"function support_switch_type() { return %d;}\n"
		"function support_num_ephy() { return %d;}\n"
		"function support_ephy_w1000() { return %d;}\n"
		"function support_ephy_l1000() { return %d;}\n"
		"function support_2g_inic_mii() { return %d;}\n"
		"function support_2g_radio() { return %d;}\n"
		"function support_5g_radio() { return %d;}\n"
		"function support_5g_11ac() { return %d;}\n"
		"function support_5g_wid() { return %d;}\n"
		"function support_2g_wid() { return %d;}\n"
		"function support_5g_stream_tx() { return %d;}\n"
		"function support_5g_stream_rx() { return %d;}\n"
		"function support_2g_stream_tx() { return %d;}\n"
		"function support_2g_stream_rx() { return %d;}\n"
		"function support_2g_turbo_qam() { return %d;}\n"
		"function support_2g_airtimefairness() { return %d;}\n"
		"function support_5g_txbf() { return %d;}\n"
		"function support_5g_band_steering() { return %d;}\n"
		"function support_5g_mumimo() { return %d;}\n"
		"function support_sfe() { return %d;}\n"
		"function support_lan_ap_isolate() { return %d;}\n"
		"function support_5g_160mhz() { return %d;}\n",
		has_ipv6,
		has_ipv6_ppe,
		has_ipv4_ppe,
		has_peap_ssl,
		has_http_ssl,
		has_ddns_ssl,
		MIN_EXT_VLAN_VID,
		max_conn,
		has_mtd_rwfs,
		has_btn_mode,
		has_usb,
		has_usb3,
		BOARD_NUM_USB_PORTS,
		has_stor,
		has_switch_type,
		BOARD_NUM_ETH_EPHY,
		BOARD_HAS_EPHY_W1000,
		BOARD_HAS_EPHY_L1000,
		has_inic_mii,
		BOARD_HAS_2G_RADIO,
		BOARD_HAS_5G_RADIO,
		has_5g_vht,
		wid_5g,
		wid_2g,
		BOARD_NUM_ANT_5G_TX,
		BOARD_NUM_ANT_5G_RX,
		BOARD_NUM_ANT_2G_TX,
		BOARD_NUM_ANT_2G_RX,
		has_2g_turbo_qam,
		has_2g_airtimefairness,
		has_5g_txbf,
		has_5g_band_steering,
		has_5g_mumimo,
		has_sfe,
		has_lan_ap_isolate,
		has_5g_160mhz
	);

	return 0;
}

static int
ej_hardware_pins_hook(int eid, webs_t wp, int argc, char **argv)
{
#if defined (BOARD_GPIO_BTN_WPS)
	int has_but_wps = 1;
#else
	int has_but_wps = 0;
#endif
#if defined (BOARD_GPIO_BTN_FN1)
	int has_but_fn1 = 1;
#else
	int has_but_fn1 = 0;
#endif
#if defined (BOARD_GPIO_BTN_FN2)
	int has_but_fn2 = 1;
#else
	int has_but_fn2 = 0;
#endif
#if defined (BOARD_GPIO_LED_ALL)
	int has_led_all = 1;
#else
	int has_led_all = 0;
#endif
#if defined (BOARD_GPIO_LED_WAN)
	int has_led_wan = 1;
#else
	int has_led_wan = 0;
#endif
#if defined (BOARD_GPIO_LED_LAN)
	int has_led_lan = 1;
#else
	int has_led_lan = 0;
#endif
#if defined (BOARD_GPIO_LED_USB) && defined (USE_USB_SUPPORT)
	int has_led_usb = 1;
#else
	int has_led_usb = 0;
#endif
#if defined (BOARD_GPIO_LED_WIFI) || defined (BOARD_GPIO_LED_SW2G) || defined (BOARD_GPIO_LED_SW5G)
	int has_led_wif = 1;
#else
	int has_led_wif = 0;
#endif
#if defined (BOARD_GPIO_LED_POWER)
	int has_led_pwr = 1;
#else
	int has_led_pwr = 0;
#endif

	websWrite(wp,
		"function support_but_wps() { return %d;}\n"
		"function support_but_fn1() { return %d;}\n"
		"function support_but_fn2() { return %d;}\n"
		"function support_led_all() { return %d;}\n"
		"function support_led_wan() { return %d;}\n"
		"function support_led_lan() { return %d;}\n"
		"function support_led_usb() { return %d;}\n"
		"function support_led_wif() { return %d;}\n"
		"function support_led_pwr() { return %d;}\n"
		"function support_led_phy() { return %d;}\n",
		has_but_wps,
		has_but_fn1,
		has_but_fn2,
		has_led_all,
		has_led_wan,
		has_led_lan,
		has_led_usb,
		has_led_wif,
		has_led_pwr,
		BOARD_NUM_ETH_LEDS
	);

	return 0;
}

static int
openssl_util_hook(int eid, webs_t wp, int argc, char **argv)
{
	int has_openssl_util = 0;

	if (f_exists("/usr/bin/openssl") || f_exists("/opt/bin/openssl"))
		has_openssl_util = 1;

	websWrite(wp, "function openssl_util_found() { return %d;}\n", has_openssl_util);
	return 0;
}

static int
openvpn_srv_cert_hook(int eid, webs_t wp, int argc, char **argv)
{
	int has_found_cert = 0;
#if defined(APP_OPENVPN)
	int i, i_atls;
	char key_file[64];
	static const char *openvpn_server_keys[5] = {
		"ca.crt",
		"dh1024.pem",
		"server.crt",
		"server.key",
		"ta.key"
	};

	has_found_cert = 1;

	i_atls = nvram_get_int("vpns_ov_atls");

	for (i=0; i<5; i++) {
		if (!i_atls && (i == 4))
			continue;
		sprintf(key_file, "%s/%s", STORAGE_OVPNSVR_DIR, openvpn_server_keys[i]);
		if (!f_exists(key_file)) {
			has_found_cert = 0;
			break;
		}
	}
#endif
	websWrite(wp, "function openvpn_srv_cert_found() { return %d;}\n", has_found_cert);

	return 0;
}

static int openvpn_cli_cert_hook(int eid, webs_t wp, int argc, char **argv)
{
	int has_found_cert = 0;
#if defined(APP_OPENVPN)
	int i, i_auth, i_atls;
	char key_file[64];
	static const char *openvpn_client_keys[4] = {
		"ca.crt",
		"client.crt",
		"client.key",
		"ta.key"
	};

	has_found_cert = 1;

	i_auth = nvram_get_int("vpnc_ov_auth");
	i_atls = nvram_get_int("vpnc_ov_atls");

	for (i=0; i<4; i++) {
		if (i_auth == 1 && (i == 1 || i == 2))
			continue;
		if (!i_atls && (i == 3))
			continue;
		sprintf(key_file, "%s/%s", STORAGE_OVPNCLI_DIR, openvpn_client_keys[i]);
		if (!f_exists(key_file)) {
			has_found_cert = 0;
			break;
		}
	}
#endif
	websWrite(wp, "function openvpn_cli_cert_found() { return %d;}\n", has_found_cert);

	return 0;
}

static int ej_get_parameter(int eid, webs_t wp, int argc, char **argv) {
	int ret = 0;

	if (argc < 1)
		return -1;

	char *value = websGetVar(wp, argv[0], "");
	websWrite(wp, "%s", value);
	return ret;
}

static int login_state_hook(int eid, webs_t wp, int argc, char **argv) {
#if defined(USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	fill_login_ip(s_addr, sizeof(s_addr));

	websWrite(wp, "function login_safe() { return %d; }\n", get_login_safe());
	websWrite(wp, "function login_ip_str() { return '%s'; }\n", s_addr);
	websWrite(wp, "function login_mac_str() { return '%s'; }\n", get_login_mac());

	return 0;
}

static int ej_get_nvram_list(int eid, webs_t wp, int argc, char **argv) {
	struct variable *v, *gv;
	char buf[NVRAM_MAX_VALUE_LEN];
	char *serviceId, *groupName, *hiddenVar;
	int i, groupCount, sid;
	int firstRow, firstItem;

	if (argc < 2)
		return 0;

	serviceId = argv[0];
	groupName = argv[1];

	hiddenVar = NULL;
	if (argc > 2 && *argv[2])
		hiddenVar = argv[2];

	sid = LookupServiceId(serviceId);
	if (sid == -1)
		return 0;

	/* Validate and set vairables in table order */
	for (v = GetVariables(sid); v->name != NULL; ++v)
		if (!strcmp(groupName, v->name))
			break;

	if (v->name == NULL)
		return 0;

	groupCount = nvram_get_int(v->argv[3]);

	firstRow = 1;
	for (i = 0; i < groupCount; ++i) {
		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "[");
		
		firstItem = 1;
		for (gv = (struct variable *)v->argv[0]; gv->name != NULL; ++gv) {
			if (firstItem == 1)
				firstItem = 0;
			else
				websWrite(wp, ", ");
			
			if (hiddenVar && strcmp(hiddenVar, gv->name) == 0)
				sprintf(buf, "\"%s\"", "*");
			else
				snprintf(buf, sizeof(buf), "\"%s\"", nvram_get_list_x(gv->name, i));
			websWrite(wp, "%s", buf);
		}
		
		websWrite(wp, "]");
	}

	return 0;
}

// for detect static IP's client.
static int ej_get_static_client(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char buf[512], *head, *tail, field[512];
	int i, lock, len, first_client, first_field;

	lock = file_lock("networkmap");

	first_client = 1;
	fp = fopen("/tmp/static_ip.inf", "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			if (first_client)
				first_client = 0;
			else
				websWrite(wp, ", ");
			
			len = strlen(buf);
			buf[len-1] = ',';
			head = buf;
			first_field = 1;
			for (i = 0; i < 6; ++i) {
				tail = strchr(head, ',');
				if (tail != NULL) {
					memset(field, 0, sizeof(field));
					strncpy(field, head, (tail-head));
				}
				
				if (first_field) {
					first_field = 0;
					websWrite(wp, "[");
				} else
					websWrite(wp, ", ");
				
				if (strlen(field) > 0)
					websWrite(wp, "\"%s\"", field);
				else
					websWrite(wp, "null");
				
				head = tail+1;
				
				if (i == 5)
					websWrite(wp, "]");
			}
		}
		
		fclose(fp);
	}

	file_unlock(lock);

	return 0;
}

static int ej_get_static_ccount(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char buf[32];
	int lock;
	unsigned int vcount;

	vcount = 0;

	lock = file_lock("networkmap");

	fp = fopen("/tmp/static_ip.num", "r");
	if (fp) {
		if (fgets(buf, sizeof(buf), fp))
			vcount = strtoul(buf, NULL, 0);
		fclose(fp);
	}

	file_unlock(lock);

	websWrite(wp, "%u", vcount);

	return 0;
}

static int ej_get_flash_time(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "function board_boot_time() { return %d;}\n", BOARD_BOOT_TIME+5);
	websWrite(wp, "function board_flash_time() { return %d;}\n", BOARD_FLASH_TIME);

	return 0;
}

static int ej_get_vpns_client(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	int first_client;
	char ifname[16], addr_l[64], addr_r[64], peer_name[64];

	fp = fopen("/tmp/vpns.leases", "r");
	if (!fp) {
		return 0;
	}

	first_client = 1;
	while (fscanf(fp, "%15s %63s %63s %63[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4) {
		if (first_client)
			first_client = 0;
		else
			websWrite(wp, ", ");
		
		websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\"]", addr_l, addr_r, peer_name, ifname);
	}

	fclose(fp);

	return 0;
}

struct cpu_stats {
	unsigned long long user;    // user (application) usage
	unsigned long long nice;    // user usage with "niced" priority
	unsigned long long system;  // system (kernel) level usage
	unsigned long long idle;    // CPU idle and no disk I/O outstanding
	unsigned long long iowait;  // CPU idle but with outstanding disk I/O
	unsigned long long irq;     // Interrupt requests
	unsigned long long sirq;    // Soft interrupt requests
	unsigned long long steal;   // Invol wait, hypervisor svcing other virtual CPU
	unsigned long long busy;
	unsigned long long total;
};

struct mem_stats {
	unsigned long total;    // RAM total
	unsigned long free;     // RAM free
	unsigned long buffers;  // RAM buffers
	unsigned long cached;   // RAM cached
	unsigned long sw_total; // Swap total
	unsigned long sw_free;  // Swap free
};

struct wifi_stats {
	int radio;
	int ap_guest;
};

void get_cpudata(struct cpu_stats *st)
{
	FILE *fp;
	char line_buf[256];

	fp = fopen("/proc/stat", "r");
	if (fp)
	{
		if (fgets(line_buf, sizeof(line_buf), fp))
		{
			if (sscanf(line_buf, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
				&st->user, &st->nice, &st->system, &st->idle,
				&st->iowait, &st->irq, &st->sirq, &st->steal) >= 4)
			{
				st->busy = st->user + st->nice + st->system + st->irq + st->sirq + st->steal;
				st->total = st->busy + st->idle + st->iowait;
			}
		}
		fclose(fp);
	}
}

void get_memdata(struct mem_stats *st)
{
	FILE *fp;
	char line_buf[64];

	fp = fopen("/proc/meminfo", "r");
	if (fp)
	{
		if (fgets(line_buf, sizeof(line_buf), fp) && 
		    sscanf(line_buf, "MemTotal: %lu %*s", &st->total) == 1)
		{
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "MemFree: %lu %*s", &st->free);
			
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "Buffers: %lu %*s", &st->buffers);
			
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "Cached: %lu %*s", &st->cached);
		}
		
		fclose(fp);
	}
}

void get_wifidata(struct wifi_stats *st, int is_5ghz)
{
	if (is_5ghz)
	{
#if BOARD_HAS_5G_RADIO
		st->radio = (nvram_get_int("mlme_radio_wl")) ? 1 : 0;
		if (st->radio)
			st->ap_guest = is_interface_up(IFNAME_5G_GUEST);
		else
			st->ap_guest = 0;
#else
		st->radio = 0;
		st->ap_guest = 0;
#endif
	}
	else
	{
#if BOARD_HAS_2G_RADIO
		st->radio = (nvram_get_int("mlme_radio_rt")) ? 1 : 0;
		if (st->radio)
			st->ap_guest = is_interface_up(IFNAME_2G_GUEST);
		else
			st->ap_guest = 0;
#else
		st->radio = 0;
		st->ap_guest = 0;
#endif
	}
}


#define LOAD_INT(x)	(unsigned)((x) >> 16)
#define LOAD_FRAC(x)	LOAD_INT(((x) & ((1 << 16) - 1)) * 100)

static int ej_system_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	struct sysinfo info;
	struct cpu_stats cpu;
	struct mem_stats mem;
	struct wifi_stats wifi2;
	struct wifi_stats wifi5;
	struct stat log;
	unsigned long updays, uphours, upminutes;

	get_cpudata(&cpu);
	get_memdata(&mem);
	get_wifidata(&wifi2, 0);
	get_wifidata(&wifi5, 1);

	sysinfo(&info);
	updays = (unsigned long) info.uptime / (unsigned long)(60*60*24);
	upminutes = (unsigned long) info.uptime / (unsigned long)60;
	uphours = (upminutes / (unsigned long)60) % (unsigned long)24;
	upminutes %= 60;

	mem.sw_total = (unsigned long)(((unsigned long long)info.totalswap * info.mem_unit) >> 10);
	mem.sw_free  = (unsigned long)(((unsigned long long)info.freeswap * info.mem_unit) >> 10);

	if (stat("/tmp/syslog.log", &log) != 0)
		log.st_mtime = 0;

	websWrite(wp, "{ lavg: \"%u.%02u %u.%02u %u.%02u\", "
			"uptime: {days: %lu, hours: %lu, minutes: %lu}, "
			"ram: {total: %lu, used: %lu, free: %lu, buffers: %lu, cached: %lu}, "
			"swap: {total: %lu, used: %lu, free: %lu}, "
			"cpu: {busy: 0x%llx, user: 0x%llx, nice: 0x%llx, system: 0x%llx, "
			      "idle: 0x%llx, iowait: 0x%llx, irq: 0x%llx, sirq: 0x%llx, total: 0x%llx}, "
			"wifi2: {state: %d, guest: %d}, "
			"wifi5: {state: %d, guest: %d}, "
			"logmt: %ld }",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]),
			updays, uphours, upminutes,
			mem.total, (mem.total - mem.free), mem.free, mem.buffers, mem.cached,
			mem.sw_total, (mem.sw_total - mem.sw_free), mem.sw_free,
			cpu.busy, cpu.user, cpu.nice, cpu.system, cpu.idle, cpu.iowait, cpu.irq, cpu.sirq, cpu.total,
			wifi2.radio, wifi2.ap_guest,
			wifi5.radio, wifi5.ap_guest,
			log.st_mtime
		);

	return 0;
}

static int ej_dump_syslog_hook(int eid, webs_t wp, int argc, char **argv)
{
	int log_lines = 0;
	int log_float = nvram_get_int("log_float_ui");

	if (log_float > 0) {
		FILE *fp;
		int lskip = 0;
		char buf[MAX_FILE_LINE_SIZE];
		
		fp = fopen("/tmp/syslog.log", "r");
		if (fp) {
			if (log_float > 1){
				int ltotal = 0;
				
				while (fgets(buf, sizeof(buf), fp)!=NULL)
					ltotal++;
				fseek(fp, 0L, SEEK_SET);
				
				lskip = ltotal - 100;
				if (lskip < 0)
					lskip = 0;
			}
			
			while (fgets(buf, sizeof(buf), fp)!=NULL){
				if (lskip > 0) {
					lskip--;
					continue;
				}
				fputs(buf, wp);
				log_lines++;
			}
			
			fclose(fp);
		}
	}

	if (!log_lines)
		fputs("", wp);

	fflush(wp);

	return 0;
}

static int ej_dump_eth_mib_hook(int eid, webs_t wp, int argc, char **argv)
{
	int eth_port_id = 0;
	const char *port_id = get_cgi("port_id");

	if (port_id)
		eth_port_id = atoi(port_id);

	if (eth_port_id < 0)
		eth_port_id = 0;

	return fill_eth_status(eth_port_id, wp);
}

#define MAX_DICT_LANGS (15)
int ej_shown_language_option(int eid, webs_t wp, int argc, char **argv) {
	FILE *fp;
	int i, len;
	const struct language_table *pLang;
	char buffer[256], key[16], target[32];
	char *follow_info, *follow_info_end, *selected, *lang_set;

	lang_set = nvram_safe_get("preferred_lang");

	fp = fopen("EN.dict", "r");
	if (!fp) {
		fprintf(stderr, "No English dictionary!\n");
		return 0;
	}

	for (i = 0; i <= MAX_DICT_LANGS; ++i) {
		follow_info = fgets(buffer, sizeof(buffer), fp);
		if (!follow_info)
			break;
		
		if (strncmp(follow_info, "LANG_", 5))    // 5 = strlen("LANG_")
			continue;
		
		follow_info += 5;
		follow_info_end = strstr(follow_info, "=");
		len = follow_info_end-follow_info;
		if (len < 1 || len >= sizeof(key))
			continue;
		
		strncpy(key, follow_info, len);
		key[len] = 0;
		
		follow_info = follow_info_end+1;
		follow_info_end = strstr(follow_info, "\n");
		len = follow_info_end-follow_info;
		if (len < 1)
			continue;
		
		if (len >= sizeof(target))
			len = sizeof(target) - 1;
		strncpy(target, follow_info, len);
		target[len] = 0;
		
		for (pLang = language_tables; pLang->Lang != NULL; ++pLang) {
			if (strcmp(key, pLang->Target_Lang))
				continue;
			
			selected = "";
			if (!strcmp(key, lang_set))
				selected = " selected";
			
			websWrite(wp, "<option value=\"%s\"%s>%s</option>\\n", key, selected, target);
			
			break;
		}
	}

	fclose(fp);

	return 0;
}

static int
apply_cgi(const char *url, webs_t wp)
{
	char *value;
	char *current_url;
	char *next_url;
	char *script;

	value = websGetVar(wp, "action_mode","");
	current_url = websGetVar(wp, "current_page", "");
	next_url = websGetVar(wp, "next_page", "");
	script = websGetVar(wp, "action_script","");

	snprintf(next_host, sizeof(next_host), "%s", websGetVar(wp, "next_host", ""));

	if (!strcmp(value, " SystemCmd "))
	{
		size_t cmd_len;
		char *cmd_str = websGetVar(wp, "SystemCmd", "");
		
		cmd_len = MIN(sizeof(SystemCmd)-1, strlen(cmd_str));
		strncpy(SystemCmd, cmd_str, cmd_len);
		SystemCmd[cmd_len] = '\0';
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value, " ClearLog "))
	{
		// current only syslog implement this button
		unlink("/tmp/syslog.log");
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value, " Reboot "))
	{
		sys_reboot();
		return 0;
	}
	else if (!strcmp(value, " Shutdown "))
	{
		system("shutdown");
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value, " RestoreNVRAM "))
	{
		websApply(wp, "Restarting.asp");
		nvram_set_int("restore_defaults", 1);
		nvram_commit();
		sys_reboot();
		return 0;
	}
	else if (!strcmp(value, " RestoreStorage "))
	{
		doSystem("/sbin/mtd_storage.sh %s", "reset");
		return 0;
	}
	else if (!strcmp(value, " CommitFlash "))
	{
		int commit_all = 0, sys_result = 0;
		char *action_id = websGetVar(wp, "nvram_action", "");
		if (strcmp(action_id, "commit_all") == 0)
			commit_all = 1;
		if (commit_all || strcmp(action_id, "commit_nvram") == 0)
			sys_result |= nvram_commit();
		if (commit_all || strcmp(action_id, "commit_storage") == 0)
			sys_result |= doSystem("/sbin/mtd_storage.sh %s", "save");
		websWrite(wp, "{\"sys_result\": %d}", sys_result);
		return 0;
	}
	else if (!strcmp(value, " ExportConfOVPNC "))
	{
		int sys_result = 1;
#if defined(APP_OPENVPN)
		char *common_name = websGetVar(wp, "common_name", "");
		int rsa_bits = atoi(websGetVar(wp, "rsa_bits", "1024"));
		int days_valid = atoi(websGetVar(wp, "days_valid", "365"));
		if (strlen(common_name) < 1)
			common_name = "client@ovpn";
		if (get_login_safe())
			sys_result = doSystem("/sbin/ovpn_export_client '%s' %d %d", common_name, rsa_bits, days_valid);
#endif
		websWrite(wp, "{\"sys_result\": %d}", sys_result);
		return 0;
	}
	else if (!strcmp(value, " CreateCertOVPNS "))
	{
		int sys_result = 1;
#if defined(APP_OPENVPN)
		char *common_name = websGetVar(wp, "common_name", "");
		int rsa_bits = atoi(websGetVar(wp, "rsa_bits", "1024"));
		int days_valid = atoi(websGetVar(wp, "days_valid", "365"));
		if (strlen(common_name) < 1)
			common_name = "OpenVPN Server";
		if (get_login_safe())
			sys_result = doSystem("/usr/bin/openvpn-cert.sh %s -n '%s' -b %d -d %d", "server", common_name, rsa_bits, days_valid);
#endif
		websWrite(wp, "{\"sys_result\": %d}", sys_result);
		return 0;
	}
	else if (!strcmp(value, " CreateCertHTTPS "))
	{
		int sys_result = 1;
#if defined(SUPPORT_HTTPS)
		char *common_name = websGetVar(wp, "common_name", "");
		int rsa_bits = atoi(websGetVar(wp, "rsa_bits", "1024"));
		int days_valid = atoi(websGetVar(wp, "days_valid", "365"));
		if (strlen(common_name) < 1)
			common_name = nvram_safe_get("lan_ipaddr_t");
		if (get_login_safe())
			sys_result = doSystem("/usr/bin/https-cert.sh -n '%s' -b %d -d %d", common_name, rsa_bits, days_valid);
#endif
		websWrite(wp, "{\"sys_result\": %d}", sys_result);
		return 0;
	}
	else
	{
		char *sid_list, *serviceId;
		int sid;
		
		sid_list = websGetVar(wp, "sid_list", "");
		while ((serviceId = svc_pop_list(sid_list, ';')) != NULL) {
			sid = 0;
			while (GetServiceId(sid) != NULL) {
				if (!strcmp(GetServiceId(sid), serviceId))
					break;
				++sid;
			}
			
			if (!strcmp(value, "  Save  ") || !strcmp(value, " Apply "))
				validate_cgi(wp, sid);
			else if (!strcmp(value, "Set") || !strcmp(value, "Unset") || !strcmp(value, "Update"))
				validate_cgi(wp, sid);
			else {
				char *value1 = websGetVar(wp, "action_mode", NULL);
				if (value1 != NULL) {
					char groupId[64];
					snprintf(groupId, sizeof(groupId), websGetVar(wp, "group_id", ""));
					
					if (!strncmp(value1, " Delete ", 8))
						apply_cgi_group(wp, sid, NULL, groupId, GROUP_FLAG_DELETE);
					else if (!strncmp(value1, " Add ", 5))
						apply_cgi_group(wp, sid, NULL, groupId, GROUP_FLAG_ADD);
					else if (!strncmp(value1, " Del ", 5))
						apply_cgi_group(wp, sid, NULL, groupId, GROUP_FLAG_REMOVE);
					else if (!strncmp(value1, " Refresh ", 9))
						apply_cgi_group(wp, sid, NULL, groupId, GROUP_FLAG_REFRESH);
					
					validate_cgi(wp, sid);
				}
			}
			
			sid_list = sid_list+strlen(serviceId)+1;
		}
		
		/* Add for EMI Test page */
		if (strlen(script) > 0)
			sys_script(script);
		
		if (!strcmp(value, "  Save  ") || !strcmp(value, " Apply "))
			websRedirect(wp, next_url);
		else
			websRedirect(wp, current_url);
		
		return 0;
	}

	return 1;
}

static void
nvram_clr_group_temp(struct variable *v)
{
	char name[64];
	struct variable *gv;

	if (v->argv[0]==NULL)
		return;

	for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++) {
		snprintf(name, sizeof(name), "%s_0", gv->name);
		
		/* clear last deleted value */
		nvram_unset(name);
	}
}

static void
nvram_add_group_item(webs_t wp, struct variable *v)
{
	char name[64], *value;
	struct variable *gv;
	int gcount;

	if (v->argv[0]==NULL)
		return;

	gcount = nvram_get_int(v->argv[3]);
	if (gcount < 0)
		gcount = 0;

	for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++) {
		snprintf(name, sizeof(name), "%s_0", gv->name);
		
		/* clear last deleted value */
		nvram_unset(name);
		
		value = websGetVar(wp, name, "");
		snprintf(name, sizeof(name), "%s%d", gv->name, gcount);
		nvram_set(name, value);
	}

	gcount++;
	nvram_set_int(v->argv[3], gcount);
}

static void
nvram_remove_group_item(struct variable *v, int *delMap)
{
	struct variable *gv;
	int i, di, gcount;

	if (v->argv[0]==NULL)
		return;

	gcount = nvram_get_int(v->argv[3]);
	if (gcount < 1)
		return;

	for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++)
		nvram_del_list_map_x(gv->name, gcount, delMap);

	di = 0;
	for (i=0; i < gcount; i++) {
		if (delMap[di]==i) {
			di++;
			if (di > MAX_GROUP_COUNT)
				break;
		}
	}

	if (di > 0) {
		gcount -= di;
		if (gcount < 0)
			gcount = 0;
		nvram_set_int(v->argv[3], gcount);
	}
}

/* Rule for table: 
 * 1. First field can not be NULL, or it is treat as empty
 * 2. 
 */

static int 
nvram_add_group_table(webs_t wp, char *serviceId, struct variable *v, int count)
{
    struct variable *gv;
    char buf[NVRAM_MAX_VALUE_LEN];
    char bufs[NVRAM_MAX_VALUE_LEN*2];
    int i, j, fieldLen, rowLen, fieldCount, value;
    int addlen=0;

    if (v->argv[0]==NULL) 
    {
       return 0;
    }

    bufs[0] = 0x0;
    rowLen = atoi(v->argv[2]);

    if (count==-1)
    {
       for (i=0;i<rowLen;i++)
       {
           bufs[i] = ' ';
       }
       value = -1;
       bufs[i] = 0x0;

       goto ToHTML;
    }

    fieldCount = 0;

    value = count;

    for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++)
    {
    	strcpy(buf, nvram_get_list_x(gv->name, count));
    	addlen=0;
    	
    	fieldLen = atoi(gv->longname)+addlen;
    	rowLen+=addlen;
    	
    	if (fieldLen!=0)
    	{
    	   if (strlen(buf)>fieldLen)
    	   {
    	      buf[fieldLen] = 0x0;
    	   }
    	   else
    	   {
    	      i = strlen(buf);
    	      j = i;
    	      for (;i<fieldLen;i++)
    	      {
    		 buf[j++] = ' ';
    	      }
    	      buf[j] = 0x0;
    	   }
    	}
    	
    	if (fieldCount==0)
    	   sprintf(bufs, "%s", buf);
    	else
    	   snprintf(bufs, sizeof(bufs), "%s%s", bufs, buf);
    	
    	fieldCount++;
    }

    if (strlen(bufs) > rowLen)
       bufs[rowLen] = 0x0;

ToHTML:
    /* Replace ' ' to &nbsp; */
    buf[0] = 0;
    j = 0;

    for (i=0; i<strlen(bufs);i++)
    {
    	if (bufs[i] == ' ')
    	{
    	   buf[j++] = '&';
    	   buf[j++] = 'n';
    	   buf[j++] = 'b';
    	   buf[j++] = 's';
    	   buf[j++] = 'p';
    	   buf[j++] = ';';
    	}
    	else buf[j++] = bufs[i];
    }
    buf[j] = 0x0;

    return (websWrite(wp, "<option value=\"%d\">%s</option>", value, buf));
}

static int
apply_cgi_group(webs_t wp, int sid, struct variable *var, const char *groupName, int flag)
{
	struct variable *v;

	if (var != NULL) {
		v = var;
	} else {
		/* Validate and set vairables in table order */
		for (v = GetVariables(sid); v->name != NULL; v++) {
			if (!strcmp(groupName, v->name))
				break;
		}
	}

	if (v->name == NULL)
		return 0;

	if (flag == GROUP_FLAG_ADD) {
		nvram_add_group_item(wp, v);
		return 1;
	} else if (flag == GROUP_FLAG_REMOVE) {
		nvram_remove_group_item(v, group_del_map);
		return 1;
	}

	return 0;
}

static int
nvram_generate_table(webs_t wp, char *serviceId, char *groupName)
{
	struct variable *v;
	int i, groupCount, ret, r, sid;

	sid = LookupServiceId(serviceId);
	if (sid == -1)
		return 0;

	/* Validate and set vairables in table order */
	for (v = GetVariables(sid); v->name != NULL; v++) {
		if (!strcmp(groupName, v->name))
			break;
	}

	if (v->name == NULL)
		return 0;

	groupCount = nvram_get_int(v->argv[3]);

	if (groupCount == 0) {
		ret = nvram_add_group_table(wp, serviceId, v, -1);
	} else {
		ret = 0;
		for (i=0; i<groupCount; i++) {
			r = nvram_add_group_table(wp, serviceId, v, i);
			if (r != 0)
				ret += r;
			else
				break;
		}
	}

	return (ret);
}

void
do_cgi_clear(void)
{
	init_cgi(NULL);
	post_buf[0] = 0;
}

void
do_uncgi_query(const char *query)
{
	size_t query_len;

	init_cgi(NULL);

	query_len = MIN(strlen(query), sizeof(post_buf)-1);
	if (query_len > 0)
		strncpy(post_buf, query, query_len);
	post_buf[query_len] = 0;

	if (strlen(post_buf) > 0)
		init_cgi(post_buf);
}

static void
do_html_apply_post(const char *url, FILE *stream, int clen, char *boundary)
{
	init_cgi(NULL);

	group_del_map[0] = -1;

	post_buf[0] = 0;
	if (!fgets(post_buf, MIN(clen+1, sizeof(post_buf)), stream))
		return;

	clen -= strlen(post_buf);
	while (clen--)
		fgetc(stream);

	websScan(post_buf);
	init_cgi(post_buf);
}

static void
do_update_cgi(const char *url, FILE *stream)
{
	struct ej_handler *handler;
	const char *pattern;
	int argc;
	char args[32];
	char *argv[16];

	pattern = get_cgi("output");
	if (pattern != NULL) {
		for (handler = &ej_handlers[0]; handler->pattern; handler++) {
			if (strcmp(handler->pattern, pattern) == 0) {
				for (argc = 0; argc < 16; ++argc) {
					sprintf(args, "arg%d", argc);
					if ((argv[argc] = (char *)get_cgi(args)) == NULL)
						break;
				}
				handler->output(0, stream, argc, argv);
				break;
			}
		}
	}
}

static void
do_apply_cgi(const char *url, FILE *stream)
{
	apply_cgi(url, stream);
}

static void
do_upgrade_fw_cgi(const char *url, FILE *stream)
{
	if (f_exists(FW_IMG_NAME) && get_login_safe()) {
		notify_rc("flash_firmware");
		websApply(stream, "Updating.asp");
	} else {
		unlink(FW_IMG_NAME);
		websApply(stream, "UpdateError.asp");
	}
}

static void
do_restore_nv_cgi(const char *url, FILE *stream)
{
	char *upload_file = PROFILE_FIFO_UPLOAD;
	int ret = -1;

	if (f_exists(upload_file) && get_login_safe()) {
		doSystem("killall %s %s", "-q", "watchdog");
		sleep(1);
		ret = eval("/usr/sbin/nvram", "restore", upload_file);
		if (ret != 0) {
			httpd_log("%s: Unable to extract %s file!", "NVRAM restore", "profile");
			eval("/sbin/watchdog");
		} else
			nvram_commit();
	}

	unlink(upload_file);

	/* Reboot if successful */
	if (ret == 0) {
		websApply(stream, "Uploading.asp");
		sys_reboot();
	} else {
		websApply(stream, "UploadError.asp");
	}
}

static void
do_restore_st_cgi(const char *url, FILE *stream)
{
	const char *upload_file = STORAGE_FIFO_FILENAME;
	int ret = -1;

	if (f_exists(upload_file) && get_login_safe()) {
		ret = eval("/sbin/mtd_storage.sh", "restore");
	}

	unlink(upload_file);

	if (ret == 0) {
		websApply(stream, "UploadDone.asp");
	} else {
		websApply(stream, "UploadError.asp");
	}
}

static void
do_nvram_file(const char *url, FILE *stream)
{
	char *nvram_file = PROFILE_FIFO_DOWNLOAD;

	unlink(nvram_file);
	if (get_login_safe()) {
		eval("/usr/sbin/nvram", "save", nvram_file);
		do_file(nvram_file, stream);
		unlink(nvram_file);
	}
}

static void
do_storage_file(const char *url, FILE *stream)
{
	char *storage_file = STORAGE_FIFO_FILENAME;

	unlink(storage_file);
	if (get_login_safe()) {
		eval("/sbin/mtd_storage.sh", "backup");
		do_file(storage_file, stream);
		unlink(storage_file);
	}
}

static void
do_syslog_file(const char *url, FILE *stream)
{
	dump_file(stream, "/tmp/syslog.log");
	fputs("\r\n", stream); /* terminator */
}

#if defined(APP_OPENVPN)
static void
do_export_ovpn_client(const char *url, FILE *stream)
{
	const char *tmp_ovpn_conf = "/tmp/client.ovpn";

	if (get_login_safe())
		do_file(tmp_ovpn_conf, stream);
	unlink(tmp_ovpn_conf);
}
#endif

static char syslog_txt[] =
"Content-Disposition: attachment;\r\n"
"filename=syslog.txt"
;

static char no_cache_IE[] =
"X-UA-Compatible: IE=edge\r\n"
"Cache-Control: no-store, no-cache, must-revalidate\r\n"
"Pragma: no-cache\r\n"
"Expires: -1"
;

#if defined (APP_SCUT)
static void
do_scutclient_log_file(const char *url, FILE *stream)
{
	dump_file(stream, "/tmp/scutclient.log");
	fputs("\r\n", stream); /* terminator */
}

static char scutclient_log_txt[] =
"Content-Disposition: attachment;\r\n"
"filename=scutclient.log"
;

#endif

#if defined (APP_MENTOHUST)
static void
do_mentohust_log_file(const char *url, FILE *stream)
{
	dump_file(stream, "/tmp/mentohust.log");
	fputs("\r\n", stream); /* terminator */
}

static char mentohust_log_txt[] =
"Content-Disposition: attachment;\r\n"
"filename=mentohust.log"
;

#endif

struct mime_handler mime_handlers[] = {
	/* cached javascript files w/o translations */
	{ "jquery.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**bootstrap.min.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**engage.itoggle.min.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**highcharts.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**highcharts_theme.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**formcontrol.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "itoggle.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "modem_isp.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "client_function.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "disk_functions.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "md5.js", "text/javascript", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23

	/* cached css  */
	{ "**.css", "text/css", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23

#if defined(APP_ARIA)
	/* cached font */
	{ "**.woff", "application/font-woff", NULL, NULL, do_file, 0 }, // 2016.01 Volt1
	{ "**.woff2", "application/font-woff", NULL, NULL, do_file, 0 },
#endif

	/* cached images */
	{ "**.png", "image/png", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**.gif", "image/gif", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**.jpg", "image/jpeg", NULL, NULL, do_file, 0 }, // 2012.06 Eagle23
	{ "**.ico", "image/x-icon", NULL, NULL, do_file, 0 }, // 2013.04 Eagle23
	{ "**.svg", "image/svg+xml", NULL, NULL, do_file, 0 }, // 2016.04 Volt1

	/* no-cached html/asp files with translations */
	{ "**.htm*", "text/html", no_cache_IE, do_html_apply_post, do_ej, 1 },
	{ "**.asp*", "text/html", no_cache_IE, do_html_apply_post, do_ej, 1 },

	/* no-cached javascript files with translations */
	{ "**.js",  "text/javascript", no_cache_IE, NULL, do_ej, 1 },

	/* downloads objects */
	{ "Settings_**.CFG", "application/force-download", NULL, NULL, do_nvram_file, 1 },
	{ "Storage_**.TBZ", "application/force-download", NULL, NULL, do_storage_file, 1 },
	{ "syslog.txt", "application/force-download", syslog_txt, NULL, do_syslog_file, 1 },
#if defined(APP_SCUT)
	{ "scutclient.log", "application/force-download", scutclient_log_txt, NULL, do_scutclient_log_file, 1 },
#endif
#if defined(APP_MENTOHUST)
	{ "mentohust.log", "application/force-download", mentohust_log_txt, NULL, do_mentohust_log_file, 1 },
#endif
#if defined(APP_OPENVPN)
	{ "client.ovpn", "application/force-download", NULL, NULL, do_export_ovpn_client, 1 },
#endif

	/* no-cached POST objects */
	{ "update.cgi*", "text/javascript", no_cache_IE, do_html_apply_post, do_update_cgi, 1 },
	{ "apply.cgi*", "text/html", no_cache_IE, do_html_apply_post, do_apply_cgi, 1 },

	{ "upgrade.cgi*",    "text/html", no_cache_IE, do_upgrade_fw_post, do_upgrade_fw_cgi, 1 },
	{ "restore_nv.cgi*", "text/html", no_cache_IE, do_restore_nv_post, do_restore_nv_cgi, 1 },
	{ "restore_st.cgi*", "text/html", no_cache_IE, do_restore_st_post, do_restore_st_cgi, 1 },

	{ NULL, NULL, NULL, NULL, NULL, 0 }
};

// traffic monitor
static int
ej_netdev(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char buf[256];
	uint64_t rx, tx;
	char *p, *ifname, comma;
	const char *ifdesc;
	int is_ap_mode, ifindex;
#if !defined(RSTATS_SKIP_ESW)
	unsigned int i;
#endif

	is_ap_mode = get_ap_mode();

	comma = ' ';

	fprintf(wp, "\nnetdevs = {\n");
	fp = fopen("/proc/net/dev", "r");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		fgets(buf, sizeof(buf), fp);
		while (fgets(buf, sizeof(buf), fp)) {
			if ((p = strchr(buf, ':')) == NULL)
				continue;
			*p = 0;
			if ((ifname = strrchr(buf, ' ')) == NULL)
				ifname = buf;
			else
				++ifname;
			
			if (strcmp(ifname, "lo") == 0)
				continue;
			
			ifindex = 0;
			ifdesc = get_ifname_descriptor(ifname, is_ap_mode, &ifindex, NULL);
			if (!ifdesc)
				continue;
			
			if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &rx, &tx) != 2)
				continue;
			
			fprintf(wp, "%c'%s':{id:%d,rx:0x%llx,tx:0x%llx}\n", comma, ifdesc, ifindex, rx, tx);
			if (comma != ',')
				comma = ',';
		}
		fclose(fp);
	}

#if !defined(RSTATS_SKIP_ESW)
	for (i = 0; i < BOARD_NUM_ETH_EPHY; i++) {
		if (get_eth_port_bytes(i, &rx, &tx) < 0)
			continue;
		fprintf(wp, "%c'ESW_P%d':{id:%d,rx:0x%llx,tx:0x%llx}\n", comma, i, 0, rx, tx);
		if (comma != ',')
			comma = ',';
	}
#endif

	fprintf(wp, "};\n");

	fflush(wp);

	return 0;
}

static int
ej_bandwidth(int eid, webs_t wp, int argc, char **argv)
{
	char *netdev;
	const char *fname = RSTATS_JS_SPEED, *nvkey = RSTATS_NVKEY_24;
	int bw_id = 0, sig = SIGUSR1;

	if (argc < 1)
		return 0;

	netdev = "";
	if (argc > 1)
		netdev = argv[1];

	if (strcmp(argv[0], "history") == 0) {
		bw_id = 1;
		sig = SIGUSR2;
		fname = RSTATS_JS_HISTORY;
		nvkey = RSTATS_NVKEY_DM;
	}

	if (strlen(netdev) > 1)
		nvram_set_temp(nvkey, netdev);

	unlink(fname);
	if (kill_pidfile_s(RSTATS_PID_FILE, sig) == 0)
		f_wait_exists(fname, 5);

	if (f_exists(fname)) {
		do_f(fname, wp);
	} else {
		if (bw_id == 0) {
			websWrite(wp,
				"\nnetdev = '%s';\n"
				"speed_history = {'%s': {}};\n"
				"data_period = %d;\n"
				"poll_next = %d;\n",
				IFDESC_LAN,
				IFDESC_LAN,
				RSTATS_INTERVAL,
				RSTATS_INTERVAL
			);
		} else {
			websWrite(wp,
				"\nnetdev = '%s';\n"
				"netdevs = ['%s'];\n"
				"daily_history = [];\n"
				"monthly_history = [];\n"
				"poll_next = %d;\n",
				IFDESC_WAN,
				IFDESC_WAN,
				RSTATS_INTERVAL
			);
		}
	}

	return 0;
}

static int
ej_backup_nvram(int eid, webs_t wp, int argc, char **argv)
{
	char *list;
	char *p, *k;
	const char *v;

	if ((argc != 1) || ((list = strdup(argv[0])) == NULL))
		return 0;

	websWrite(wp, "\nnvram = {\n");
	p = list;
	while ((k = strsep(&p, ",")) != NULL) {
		if (*k == 0)
			continue;
		v = nvram_safe_get(k);
		websWrite(wp, "\t%s: '", k);
		websWrite(wp, v);
		websWrite(wp, "',\n");
	}
	free(list);
	websWrite(wp, "\thttp_id: '");
	websWrite(wp, nvram_safe_get("http_id"));
	websWrite(wp, "'};\n");

	return 0;
}

#if !defined (USE_USB_SUPPORT)
static int
ej_get_usb_ports_info(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "function get_usb_ports_num(){return %u;}\n", 0);
	websWrite(wp, "function get_device_type_usb(port_num){return \"unknown\";}\n");
	websWrite(wp, "function printer_ports(){return [];}\n");
	websWrite(wp, "function modem_ports(){return [];}\n");
	websWrite(wp, "function modem_devnum(){return [];}\n");
	return 0;
}
#endif

static int
ej_get_ext_ports_info(int eid, webs_t wp, int argc, char **argv)
{
#if defined (USE_ATA_SUPPORT)
	int i_ata_support = 1;
#else
	int i_ata_support = 0;
#endif
#if defined (USE_MMC_SUPPORT)
	int i_mmc_support = 1;
#else
	int i_mmc_support = 0;
#endif
	websWrite(wp, "function get_ata_support(){return %d;}\n", i_ata_support);
	websWrite(wp, "function get_mmc_support(){return %d;}\n", i_mmc_support);
	return 0;
}

#if !defined (USE_STORAGE)
static int
ej_disk_pool_mapping_info(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "%s", initial_disk_pool_mapping_info());
	return 0;
}

static int
ej_available_disk_names_and_sizes(int eid, webs_t wp, int argc, char **argv)
{
	websWrite(wp, "%s", initial_blank_disk_names_and_sizes());
	websWrite(wp, "%s", initial_available_disk_names_and_sizes());
	return 0;
}
#endif

struct ej_handler ej_handlers[] =
{
	{ "nvram_get_x", ej_nvram_get_x},
	{ "nvram_get_list_x", ej_nvram_get_list_x},
	{ "nvram_get_buf_x", ej_nvram_get_buf_x},
	{ "nvram_get_table_x", ej_nvram_get_table_x},
	{ "nvram_match_x", ej_nvram_match_x},
	{ "nvram_double_match_x", ej_nvram_double_match_x},
	{ "nvram_match_both_x", ej_nvram_match_both_x},
	{ "nvram_match_list_x", ej_nvram_match_list_x},
	{ "uptime", ej_uptime},
	{ "nvram_dump", ej_dump},
	{ "firmware_caps_hook", ej_firmware_caps_hook},
	{ "json_system_status", ej_system_status_hook},

	{ "netdev", ej_netdev},
	{ "bandwidth", ej_bandwidth},
	{ "nvram", ej_backup_nvram},

	{ "nvram_get_ddns", ej_nvram_get_ddns},
	{ "nvram_char_to_ascii", ej_nvram_char_to_ascii},
	{ "update_variables", update_variables_ex},
	{ "asus_nvram_commit", asus_nvram_commit},
	{ "notify_services", ej_notify_services},
	{ "login_state_hook", login_state_hook},
	{ "wanlink", wanlink_hook},
	{ "lanlink", lanlink_hook},
	{ "wan_action", wan_action_hook},
	{ "wol_action", wol_action_hook},
	{ "nf_values", nf_values_hook},
	{ "get_parameter", ej_get_parameter},
	{ "get_nvram_list", ej_get_nvram_list},
	{ "get_flash_time", ej_get_flash_time},
	{ "get_static_client", ej_get_static_client},
	{ "get_static_ccount", ej_get_static_ccount},
#ifndef WEBUI_HIDE_VPN
	{ "get_vpns_client", ej_get_vpns_client},
#endif
	{ "wl_auth_list", ej_wl_auth_list},
#if BOARD_HAS_5G_RADIO
	{ "wl_scan_5g", ej_wl_scan_5g},
	{ "wl_bssid_5g", ej_wl_bssid_5g},
#endif
	{ "wl_bssid_2g", ej_wl_bssid_2g},
	{ "wl_scan_2g", ej_wl_scan_2g},
	{ "shown_language_option", ej_shown_language_option},
	{ "hardware_pins", ej_hardware_pins_hook},
	{ "detect_internet", ej_detect_internet_hook},
	{ "dump_syslog", ej_dump_syslog_hook},
	{ "dump_eth_mib", ej_dump_eth_mib_hook},
	{ "get_usb_ports_info", ej_get_usb_ports_info},
	{ "get_ext_ports_info", ej_get_ext_ports_info},
	{ "disk_pool_mapping_info", ej_disk_pool_mapping_info},
	{ "available_disk_names_and_sizes", ej_available_disk_names_and_sizes},
#if defined (USE_STORAGE)
	{ "get_usb_share_list", ej_get_storage_share_list},
	{ "get_AiDisk_status", ej_get_AiDisk_status},
	{ "set_AiDisk_status", ej_set_AiDisk_status},
	{ "get_all_accounts", ej_get_all_accounts},
	{ "safely_remove_disk", ej_safely_remove_disk},
	{ "get_permissions_of_account", ej_get_permissions_of_account},
	{ "set_account_permission", ej_set_account_permission},
	{ "get_folder_tree", ej_get_folder_tree},
	{ "get_share_tree", ej_get_share_tree},
	{ "initial_account", ej_initial_account},
	{ "create_account", ej_create_account},
	{ "delete_account", ej_delete_account},
	{ "modify_account", ej_modify_account},
	{ "create_sharedfolder", ej_create_sharedfolder},
	{ "delete_sharedfolder", ej_delete_sharedfolder},
	{ "modify_sharedfolder", ej_modify_sharedfolder},
	{ "set_share_mode", ej_set_share_mode},
#endif
#if defined (APP_SCUT)
	{ "scutclient_action", scutclient_action_hook},
	{ "scutclient_status", scutclient_status_hook},
	{ "scutclient_version", scutclient_version_hook},
#endif
#if defined (APP_MENTOHUST)
	{ "mentohust_action", mentohust_action_hook},
	{ "mentohust_status", mentohust_status_hook},
#endif
#if defined (APP_SHADOWSOCKS)
	{ "shadowsocks_action", shadowsocks_action_hook},
	{ "shadowsocks_status", shadowsocks_status_hook},
	{ "rules_count", rules_count_hook},
#endif
#if defined (APP_DNSFORWARDER)
	{ "dnsforwarder_status", dnsforwarder_status_hook},
#endif
	{ "openssl_util_hook", openssl_util_hook},
	{ "openvpn_srv_cert_hook", openvpn_srv_cert_hook},
	{ "openvpn_cli_cert_hook", openvpn_cli_cert_hook},
	{ NULL, NULL }
};

