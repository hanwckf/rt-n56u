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

typedef unsigned char   bool;

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
#include <sys/mman.h>

#ifndef __user
#define __user
#endif

#include <wireless.h>
#include <ralink_priv.h>
#include <notify_rc.h>
#include <bin_sem_asus.h>

#include "common.h"
#include "nvram_x.h"
#include "httpd.h"

#ifndef O_BINARY
#define O_BINARY	0
#endif

#include <image.h>
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#define MAX_GROUP_COUNT		64
#define GROUP_FLAG_REFRESH 	0
#define GROUP_FLAG_DELETE 	1
#define GROUP_FLAG_ADD 		2
#define GROUP_FLAG_REMOVE 	3

#define IMAGE_HEADER		"HDR0"
#define PROFILE_HEADER		"HDR1"
#define PROFILE_HEADER_NEW	"HDR2"
#define PROFILE_FIFO_UPLOAD	"/tmp/settings_u.prf"
#define PROFILE_FIFO_DOWNLOAD	"/tmp/settings_d.prf"

static int apply_cgi_group(webs_t wp, int sid, struct variable *var, char *groupName, int flag);
static int nvram_generate_table(webs_t wp, char *serviceId, char *groupName);

static int nvram_modified = 0;
static int wl_modified = 0;
static int rt_modified = 0;
static unsigned long restart_needed_bits = 0;

int action;
char *serviceId;
char *next_host;
int delMap[MAX_GROUP_COUNT+2];
char SystemCmd[128];

extern int change_passwd;
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
	dbG("[httpd] reboot...\n");

	kill(1, SIGTERM);
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

void
reltime(unsigned long seconds, char *cs)
{
#ifdef SHOWALL
	unsigned int days=0, hours=0, minutes=0;

	if (seconds > 60*60*24) {
		days = seconds / (60*60*24);
		seconds %= 60*60*24;
	}
	if (seconds > 60*60) {
		hours = seconds / (60*60);
		seconds %= 60*60;
	}
	if (seconds > 60) {
		minutes = seconds / 60;
		seconds %= 60;
	}
	sprintf(cs, "%d days, %d hours, %d minutes, %ld seconds", days, hours, minutes, seconds);
#else
	sprintf(cs, "%ld secs", seconds);
#endif
}

/******************************************************************************/
/*
 *	Redirect the user to another webs page
 */
/*
char *getip(FILE *fp)
{
	char *lan_host;
	
	if (!next_host || strlen(next_host) == 0)
	{
		lan_host = nvram_safe_get("lan_ipaddr_t");
		if (strlen(lan_host) == 0)
			lan_host = nvram_safe_get("lan_ipaddr");
		
		return lan_host;
	}
	else
	{
		return (next_host);
	}
}
*/

void websRedirect(webs_t wp, const char *url)
{
	char *http_str;

#if defined (SUPPORT_HTTPS)
	if (http_is_ssl)
		http_str = "https";
	else
#endif
	http_str = "http";

	websWrite(wp, "<html><head>\r\n");
	if (next_host && *next_host)
		websWrite(wp, "<meta http-equiv=\"refresh\" content=\"0; url=%s://%s/%s\">\r\n", http_str, next_host, url);
	else
		websWrite(wp, "<meta http-equiv=\"refresh\" content=\"0; url=%s\">\r\n", url);
	websWrite(wp, "<meta http-equiv=\"Content-Type\" content=\"text/html\">\r\n");
	websWrite(wp, "</head></html>\r\n");

	websDone(wp, 200);
}

void sys_script(char *name)
{
	char scmd[64];
	int u2ec_fifo;
#if defined(USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	sprintf(scmd, "/tmp/%s", name);

	if (strcmp(name,"syscmd.sh")==0)
	{
		if (SystemCmd[0])
		{
			char path_env[64];
			snprintf(path_env, sizeof(path_env), "PATH=%s", SYS_EXEC_PATH_OPT);
			putenv(path_env);
			doSystem("%s >/tmp/syscmd.log 2>&1\n", SystemCmd);
			SystemCmd[0] = '\0';
		}
		else
		{
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
		if (!strstr(value, "ssh-dss") && !strstr(value, "ssh-rsa"))
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

void websScan(char *str)
{
#define SCAN_MAX_VALUE_LEN 256
	unsigned int i, flag, i_len;
	char *v1, *v2, *v3, *sp;
	char groupid[64];
	char value[SCAN_MAX_VALUE_LEN];
	
	v1 = strchr(str, '?');
	
	i = 0;
	flag = 0;
	groupid[0] = 0;
	
	while (v1!=NULL)
	{
	    v2 = strchr(v1+1, '=');
	    v3 = strchr(v1+1, '&');
	    if (!v2)
		break;
	
	    if (v3)
	    {
	       i_len = v3-v2-1;
	       if (i_len > (SCAN_MAX_VALUE_LEN - 1))
	           i_len = (SCAN_MAX_VALUE_LEN - 1);
	       strncpy(value, v2+1, i_len);
	       value[i_len] = 0;
	    }
	    else
	    {
	       snprintf(value, sizeof(value), "%s", v2+1);
	    }
	
	    if (v2 != NULL && ((sp = strchr(v1+1, ' ')) == NULL || (sp > v2)))
	    {
	       if (flag && strncmp(v1+1, groupid, strlen(groupid))==0)
	       {
		   delMap[i] = atoi(value);
		   if (delMap[i]==-1)  break;
		   i++;
	       }
	       else if (strncmp(v1+1,"group_id", 8)==0)
	       {
		   snprintf(groupid, sizeof(groupid), "%s_s", value);
		   flag = 1;
	       }
	    }
	    v1 = strchr(v1+1, '&');
	}
	delMap[i] = -1;
	return;
}

void websApply(webs_t wp, char *url)
{
	do_ej (url, wp);
	websDone (wp, 200);
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

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	cn = nvram_safe_get(name);
	for (c = cn; *c; c++) {
		if (*c == 0x26 || // &
		    *c == 0x3C || // <
		    *c == 0x3E)   // >
			*c = '_';
		else
		if (*c == 0x22)   // "
			*c = ' ';
	}

	return websWrite(wp, "%s", cn);
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
	else
		return 0;
}

static int
ej_select_channel(int eid, webs_t wp, int argc, char **argv)
{
	char *sid, chstr[32];
	int ret = 0;
	int idx = 0, channel;
	char *value = nvram_safe_get("rt_country_code");
	char *channel_s = nvram_safe_get("rt_channel");
	
	if (ejArgs(argc, argv, "%s", &sid) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	channel = atoi(channel_s);

	for (idx = 0; idx < 12; idx++)
	{
		if (idx == 0)
			strcpy(chstr, "Auto");
		else
			sprintf(chstr, "%d", idx);
		ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>", idx, (idx == channel)? "selected" : "", chstr);
	}

	if (    strcasecmp(value, "CA") && strcasecmp(value, "CO") && strcasecmp(value, "DO") &&
		strcasecmp(value, "GT") && strcasecmp(value, "MX") && strcasecmp(value, "NO") &&
		strcasecmp(value, "PA") && strcasecmp(value, "PR") && strcasecmp(value, "TW") &&
		strcasecmp(value, "US") && strcasecmp(value, "UZ") )
	{
		for (idx = 12; idx < 14; idx++)
		{
			sprintf(chstr, "%d", idx);
			ret += websWrite(wp, "<option value=\"%d\" %s>%s</option>", idx, (idx == channel)? "selected" : "", chstr);
		}
	}

	if ((strcmp(value, "") == 0) || (strcasecmp(value, "DB") == 0)/* || (strcasecmp(value, "JP") == 0)*/)
		ret += websWrite(wp, "<option value=\"14\" %s>14</option>", (14 == channel)? "selected" : "");

	return ret;
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

/* Report sys up time */
static int
ej_uptime(int eid, webs_t wp, int argc, char **argv)
{
	char buf[512];
	int ret;
	char *str;
	time_t tm;

	time(&tm);
	sprintf(buf, rfctime(&tm));

	str = file2str("/proc/uptime", 64);
	if (str) {
		unsigned long up = atol(str);
		free(str);
		char lease_buf[128];
		memset(lease_buf, 0, sizeof(lease_buf));
		reltime(up, lease_buf);
		sprintf(buf, "%s(%s since boot)", buf, lease_buf);
	}

	ret = websWrite(wp, buf);
	return ret;
}

int
websWriteCh(webs_t wp, char *ch, int count)
{
   int i, ret;

   ret = 0;
   for (i=0; i<count; i++)
      ret+=websWrite(wp, "%s", ch);
   return (ret);
} 

static int dump_file(webs_t wp, char *filename)
{
	FILE *fp;
	char *extensions;
	char buf[MAX_FILE_LINE_SIZE];
	int ret = 0;

	if (!f_exists(filename)) {
		ret += websWrite(wp, "%s", "");
		return ret;
	}

	extensions = strrchr(filename, '.');
	if (extensions && strcmp(extensions, ".key") == 0) {
		ret += websWrite(wp, "%s", "# !!!This is hidden write-only secret key file!!!\n");
		return ret;
	}

	fp = fopen(filename, "r");
	if (!fp) {
		ret += websWrite(wp, "%s", "");
		return ret;
	}

	while (fgets(buf, sizeof(buf), fp)!=NULL)
		ret += websWrite(wp, buf);

	fclose(fp);

	return ret;
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

	if (strcmp(file, "wlan11b.log")==0)
		return (ej_wl_status_5g(eid, wp, 0, NULL));
	else if (strcmp(file, "wlan11b_2g.log")==0)
		return (ej_wl_status_2g(eid, wp, 0, NULL));
	else if (strcmp(file, "eth_wan.log")==0)
		return ej_eth_status_wan(eid, wp, 0, NULL);
	else if (strcmp(file, "eth_lan1.log")==0)
		return ej_eth_status_lan1(eid, wp, 0, NULL);
	else if (strcmp(file, "eth_lan2.log")==0)
		return ej_eth_status_lan2(eid, wp, 0, NULL);
	else if (strcmp(file, "eth_lan3.log")==0)
		return ej_eth_status_lan3(eid, wp, 0, NULL);
	else if (strcmp(file, "eth_lan4.log")==0)
		return ej_eth_status_lan4(eid, wp, 0, NULL);
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
		sprintf(filename, "%s/%s", STORAGE_HTTPSSL_DIR, file+8);
	else if (strncmp(file, "ovpnsvr.", 8)==0)
		sprintf(filename, "%s/%s", STORAGE_OVPNSVR_DIR, file+8);
	else if (strncmp(file, "ovpncli.", 8)==0)
		sprintf(filename, "%s/%s", STORAGE_OVPNCLI_DIR, file+8);
	else if (strncmp(file, "dnsmasq.", 8)==0)
		sprintf(filename, "%s/%s", STORAGE_DNSMASQ_DIR, file+8);
	else if (strncmp(file, "scripts.", 8)==0)
		sprintf(filename, "%s/%s", STORAGE_SCRIPTS_DIR, file+8);
	else
		sprintf(filename, "%s/%s", "/tmp", file);
	
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
    for (v = GetVariables(sid); v->name != NULL; v++) 
    {
	snprintf(name, sizeof(name), "%s", v->name);
	if ((value = websGetVar(wp, name, NULL)))
	{
		if (strcmp(v->longname, "Group") && strcmp(v->longname, "File"))
			nvram_set(v->name, value);
	}
    }
}

char *svc_pop_list(char *value, char key)
{
    char *v, *buf;
    int i;

    if (value==NULL || *value=='\0')
       return (NULL);      

    buf = value;
    v = strchr(buf, key);

    i = 0;

    if (v!=NULL)
    {
	*v = '\0';
	return (buf);
    }
    return (NULL);
}

static void do_html_post_and_get(char *url, FILE *stream, int clen, char *boundary) {
	char *query = NULL;
	static char post_buf[24576] = { 0 };
	static char post_buf_backup[24576] = { 0 };
	
	init_cgi(NULL);
	
	post_buf[0] = 0;
	post_buf_backup[0] = 0;
	
	if (fgets(post_buf, MIN(clen+1, sizeof(post_buf)), stream)) {
		clen -= strlen(post_buf);
		
		while (clen--)
			(void)fgetc(stream);
	}
	
	query = url;
	strsep(&query, "?");
	
	if (query && strlen(query) > 0) {
		if (strlen(post_buf) > 0)
			snprintf(post_buf_backup, sizeof(post_buf_backup), "?%s&%s", post_buf, query);
		else
			snprintf(post_buf_backup, sizeof(post_buf_backup), "?%s", query);
		
		snprintf(post_buf, sizeof(post_buf), "%s", post_buf_backup+1);
	}
	else if (strlen(post_buf) > 0)
		snprintf(post_buf_backup, sizeof(post_buf_backup), "?%s", post_buf);
	
	websScan(post_buf_backup);
	init_cgi(post_buf);
}

#define WIFI_IWPRIV_CHANGE_BIT	(1<<0)
#define WIFI_COMMON_CHANGE_BIT	(1<<1)
#define WIFI_RADIO_CONTROL_BIT	(1<<2)
#define WIFI_GUEST_CONTROL_BIT	(1<<3)
#define WIFI_SCHED_CONTROL_BIT	(1<<4)


static const char* wifn_list[][3] = {
	{IFNAME_2G_MAIN, IFNAME_2G_APCLI, IFNAME_2G_WDS0},
#if BOARD_HAS_5G_RADIO
	{IFNAME_5G_MAIN, IFNAME_5G_APCLI, IFNAME_5G_WDS0}
#endif
};

static char* get_wifi_ifname(int is_5g)
{
	int i;

	is_5g &= 1;
	for (i = 0; i < ARRAY_SIZE(wifn_list[is_5g]); i++) {
		char *wifn = (char *)wifn_list[is_5g][i];
		if (is_interface_up(wifn))
			return wifn;
	}

	return NULL;
}

static void set_wifi_ssid(char* ifname, char* value)
{
	doSystem("iwpriv %s set %s=\"%s\"", ifname, "SSID", value);
}

static void set_wifi_param_int(char* ifname, char* param, char* value, int val_min, int val_max)
{
	int i_value = atoi(value);
	if (i_value < val_min) i_value = val_min;
	if (i_value > val_max) i_value = val_max;

	doSystem("iwpriv %s set %s=%d", ifname, param, i_value);
}

static void set_wifi_mrate(char* ifname, char* value)
{
	int i_value = atoi(value);
	int i_mphy = 3;
	int i_mmcs = 1;

	switch (i_value)
	{
	case 0: // HTMIX (1S) 6.5-15 Mbps
		i_mphy = 3;
		i_mmcs = 0;
		break;
	case 1: // HTMIX (1S) 15-30 Mbps
		i_mphy = 3;
		i_mmcs = 1;
		break;
	case 2: // HTMIX (1S) 19.5-45 Mbps
		i_mphy = 3;
		i_mmcs = 2;
		break;
	case 3: // HTMIX (2S) 13-30 Mbps
		i_mphy = 3;
		i_mmcs = 8;
		break;
	case 4: // HTMIX (2S) 26-60 Mbps
		i_mphy = 3;
		i_mmcs = 9;
		break;
	case 5: // OFDM 9 Mbps
		i_mphy = 2;
		i_mmcs = 1;
		break;
	case 6: // OFDM 12 Mbps
		i_mphy = 2;
		i_mmcs = 2;
		break;
	case 7: // OFDM 18 Mbps
		i_mphy = 2;
		i_mmcs = 3;
		break;
	case 8: // OFDM 24 Mbps
		i_mphy = 2;
		i_mmcs = 4;
		break;
	case 9: // CCK 11 Mbps
		i_mphy = 1;
		i_mmcs = 3;
		break;
	}

	doSystem("iwpriv %s set %s=%d", ifname, "McastPhyMode", i_mphy);
	doSystem("iwpriv %s set %s=%d", ifname, "McastMcs", i_mmcs);
}

static void set_wifi_mcs_mode(char* ifname, char* value)
{
	int i_value = atoi(value);
	int i_fix = 0;  // FixedTxMode=OFF
	int i_mcs = 33; // HT_MCS=Auto

	switch (i_value)
	{
	case 1: // HTMIX (1S) 19.5-45 Mbps
		i_mcs = 2;
		break;
	case 2: // HTMIX (1S) 15-30 Mbps
		i_mcs = 1;
		break;
	case 3: // HTMIX (1S) 6.5-15 Mbps
		i_mcs = 0;
		break;
	case 4: // OFDM 12 Mbps
		i_fix = 2;
		i_mcs = 2;
		break;
	case 5: // OFDM 9 Mbps
		i_fix = 2;
		i_mcs = 1;
		break;
	case 6: // OFDM 6 Mbps
		i_fix = 2;
		i_mcs = 0;
		break;
	case 7: // CCK 5.5 Mbps
		i_fix = 1;
		i_mcs = 2;
		break;
	case 8: // CCK 2 Mbps
		i_fix = 1;
		i_mcs = 1;
		break;
	case 9: // CCK 1 Mbps
		i_fix = 1;
		i_mcs = 0;
		break;
	}

	doSystem("iwpriv %s set %s=%d", ifname, "FixedTxMode", i_fix);
	doSystem("iwpriv %s set %s=%d", ifname, "HtMcs", i_mcs);
}

static int validate_asp_apply(webs_t wp, int sid) {
	struct variable *v;
	char *value;
	char name[64];
	char buff[100];
	
	/* Validate and set variables in table order */
	for (v = GetVariables(sid); v->name != NULL; ++v) {
		snprintf(name, sizeof(name), "%s", v->name);

		if ((value = websGetVar(wp, name, NULL))) {
			if (!strcmp(v->longname, "Group")) {
				;
			} else if (!strcmp(v->longname, "File")) {
				if (!strncmp(v->name, "dnsmasq.", 8)) {
					if (write_textarea_to_file(value, STORAGE_DNSMASQ_DIR, v->name+8))
						restart_needed_bits |= v->event;
				} else if (!strncmp(v->name, "scripts.", 8)) {
					if (write_textarea_to_file(value, STORAGE_SCRIPTS_DIR, v->name+8))
						restart_needed_bits |= v->event;
				}
#if defined (SUPPORT_HTTPS)
				else if (!strncmp(v->name, "httpssl.", 8)) {
					if (write_textarea_to_file(value, STORAGE_HTTPSSL_DIR, v->name+8))
						restart_needed_bits |= v->event;
				}
#endif
#if defined(APP_OPENVPN)
				else if (!strncmp(v->name, "ovpnsvr.", 8)) {
					if (write_textarea_to_file(value, STORAGE_OVPNSVR_DIR, v->name+8))
						restart_needed_bits |= v->event;
				} else if (!strncmp(v->name, "ovpncli.", 8)) {
					if (write_textarea_to_file(value, STORAGE_OVPNCLI_DIR, v->name+8))
						restart_needed_bits |= v->event;
				}
#endif
			} else if (!strcmp(v->name, "wl_country_code")) {
				
				if ( strcmp(nvram_safe_get(name), value) ) {
					nvram_set(v->name, value);
					
					wl_modified |= WIFI_COMMON_CHANGE_BIT;
					
					nvram_modified = 1;
					
					restart_needed_bits |= v->event;
				}
				
			} else if (!strcmp(v->name, "rt_country_code")) {
				
				if ( strcmp(nvram_safe_get(name), value) ) {
					nvram_set(v->name, value);
					
					rt_modified |= WIFI_COMMON_CHANGE_BIT;
					
					nvram_modified = 1;
					
					restart_needed_bits |= v->event;
				}
				
			} else if (strcmp(nvram_safe_get(name), value) && strncmp(v->name, "wsc", 3) && strncmp(v->name, "wps", 3)) {
				
				if (!strcmp(v->name, "http_passwd") && strlen(value) == 0)
					continue;
				
				nvram_set(v->name, value);
				
				if (!strcmp(v->name, "http_username"))
				{
					change_passwd = 1;
					recreate_passwd_unix(1);
				}
				else if (!strcmp(v->name, "http_passwd"))
				{
					// only change unix password
					change_passwd = 1;
					change_passwd_unix(nvram_safe_get("http_username"), value);
				}
				
				nvram_modified = 1;
				
#if BOARD_HAS_5G_RADIO
				if (!strncmp(v->name, "wl_", 3) && strcmp(v->name, "wl_ssid2"))
				{
#if BOARD_5G_IN_SOC
/* APSoC driver has issue on direct change SSID via iwpriv */
					if (!strcmp(v->name, "wl_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("wl_ssid2", buff);
					}
#else
					if (!strcmp(v->name, "wl_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("wl_ssid2", buff);
						set_wifi_ssid(IFNAME_5G_MAIN, value);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "wl_guest_ssid"))
					{
						set_wifi_ssid(IFNAME_5G_GUEST, value);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else
#endif
					if (!strcmp(v->name, "wl_TxPower"))
					{
						char *wifn = get_wifi_ifname(1);
						if (wifn)
							set_wifi_param_int(wifn, "TxPower", value, 0, 100);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "wl_greenap"))
					{
						set_wifi_param_int(IFNAME_5G_MAIN, "GreenAP", value, 0, 1);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "wl_IgmpSnEnable"))
					{
						set_wifi_param_int(IFNAME_5G_MAIN, "IgmpSnEnable", value, 0, 1);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "wl_mcastrate"))
					{
						set_wifi_mrate(IFNAME_5G_MAIN, value);
						
						wl_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "wl_guest_mcs_mode"))
					{
						set_wifi_mcs_mode(IFNAME_5G_GUEST, value);
						
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
#if BOARD_2G_IN_SOC
/* APSoC driver has issue on direct change SSID via iwpriv */
					if (!strcmp(v->name, "rt_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("rt_ssid2", buff);
					}
#else
					if (!strcmp(v->name, "rt_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("rt_ssid2", buff);
						set_wifi_ssid(IFNAME_2G_MAIN, value);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "rt_guest_ssid"))
					{
						set_wifi_ssid(IFNAME_2G_GUEST, value);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else
#endif
					if (!strcmp(v->name, "rt_TxPower"))
					{
						char *wifn = get_wifi_ifname(0);
						if (wifn)
							set_wifi_param_int(wifn, "TxPower", value, 0, 100);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "rt_greenap"))
					{
						set_wifi_param_int(IFNAME_2G_MAIN, "GreenAP", value, 0, 1);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "rt_IgmpSnEnable"))
					{
						set_wifi_param_int(IFNAME_2G_MAIN, "IgmpSnEnable", value, 0, 1);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "rt_mcastrate"))
					{
						set_wifi_mrate(IFNAME_2G_MAIN, value);
						
						rt_modified |= WIFI_IWPRIV_CHANGE_BIT;
					}
					else if (!strcmp(v->name, "rt_guest_mcs_mode"))
					{
						set_wifi_mcs_mode(IFNAME_2G_GUEST, value);
						
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
				
				if (v->event)
				{
					restart_needed_bits |= v->event;
					dbG("debug restart_needed_bits: 0x%lx\n", restart_needed_bits);
				}
			}
		}
	}

	return (nvram_modified || restart_needed_bits) ? 1 : 0;
}

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

static int update_variables_ex(int eid, webs_t wp, int argc, char **argv) {
	int sid;
	char *action_mode;
	char *sid_list;
	char *script;
	int result;
	unsigned int restart_total_time = 0;

	restart_needed_bits = 0;

	// assign control variables
	action_mode = websGetVar(wp, "action_mode", "");
	script = websGetVar(wp, "action_script", "");
	sid_list = websGetVar(wp, "sid_list", "");

	while ((serviceId = svc_pop_list(sid_list, ';'))) {
		sid = 0;
		while (GetServiceId(sid) != NULL) {
			if (!strcmp(GetServiceId(sid), serviceId))
				break;
			
			sid++;
		}
		
		if (serviceId != NULL) {
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
						if (result == 1)
							nvram_set_int_temp(group_id, 1);
						
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
					else if (!strcmp(action_mode, " Del ")) {
						result = apply_cgi_group(wp, sid, NULL, group_id, GROUP_FLAG_REMOVE);
						if (result == 1)
							nvram_set_int_temp(group_id, 1);
						
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
					else if (!strcmp(action_mode, " Restart ")) {
						struct variable *v;
						
						for (v = GetVariables(sid); v->name != NULL; ++v) {
							if (!strcmp(v->name, group_id))
								break;
						}
						
						validate_asp_apply(wp, sid);	// for some nvram with this group
						
						if (nvram_get_int(group_id) > 0) {
							restart_needed_bits |= v->event;
							dbG("group restart_needed_bits: 0x%lx\n", restart_needed_bits);
							
							if (!strcmp(group_id, "RBRList") || !strcmp(group_id, "ACLList"))
								wl_modified |= WIFI_COMMON_CHANGE_BIT;
							if (!strcmp(group_id, "rt_RBRList") || !strcmp(group_id, "rt_ACLList"))
								rt_modified |= WIFI_COMMON_CHANGE_BIT;
							
							nvram_modified = 1;
							nvram_set_int_temp(group_id, 0);
						}
						
						if (nvram_modified)
							websWrite(wp, "<script>done_committing();</script>\n");
						else
							websWrite(wp, "<script>no_changes_and_no_committing();</script>\n");
					}
					
					validate_cgi(wp, sid);	// for some nvram with this group group
				}
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

	if (restart_needed_bits != 0 && (!strcmp(action_mode, " Apply ") || !strcmp(action_mode, " Restart ") || !strcmp(action_mode, " WPS_Apply "))) {
		if ((restart_needed_bits & RESTART_REBOOT) != 0)
			restart_total_time = MAX(ITVL_RESTART_REBOOT, restart_total_time);
		if ((restart_needed_bits & (RESTART_WAN | RESTART_IPV6)) != 0) {
			unsigned int max_time = ITVL_RESTART_WAN;
			if (nvram_match("wan_proto", "dhcp") || nvram_match("wan_proto", "static"))
				max_time = 3;
			restart_total_time = MAX(max_time, restart_total_time);
		}
		if ((restart_needed_bits & RESTART_LAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_LAN, restart_total_time);
		if ((restart_needed_bits & RESTART_DHCPD) != 0)
			restart_total_time = MAX(ITVL_RESTART_DHCPD, restart_total_time);
		if ((restart_needed_bits & RESTART_RADVD) != 0)
			restart_total_time = MAX(ITVL_RESTART_RADVD, restart_total_time);
		if ((restart_needed_bits & RESTART_MODEM) != 0)
			restart_total_time = MAX(ITVL_RESTART_MODEM, restart_total_time);
		if ((restart_needed_bits & RESTART_IPTV) != 0)
			restart_total_time = MAX(ITVL_RESTART_IPTV, restart_total_time);
		if ((restart_needed_bits & RESTART_FTPSAMBA) != 0)
			restart_total_time = MAX(ITVL_RESTART_FTPSAMBA, restart_total_time);
		if ((restart_needed_bits & RESTART_TERMINAL) != 0)
			restart_total_time = MAX(ITVL_RESTART_TERMINAL, restart_total_time);
		if ((restart_needed_bits & RESTART_VPNSVR) != 0)
			restart_total_time = MAX(ITVL_RESTART_VPNSVR, restart_total_time);
		if ((restart_needed_bits & RESTART_VPNCLI) != 0)
			restart_total_time = MAX(ITVL_RESTART_VPNCLI, restart_total_time);
		if ((restart_needed_bits & RESTART_DDNS) != 0)
			restart_total_time = MAX(ITVL_RESTART_DDNS, restart_total_time);
		if ((restart_needed_bits & RESTART_HTTPD) != 0)
			restart_total_time = MAX(ITVL_RESTART_HTTPD, restart_total_time);
		if ((restart_needed_bits & RESTART_DI) != 0)
			restart_total_time = MAX(ITVL_RESTART_DI, restart_total_time);
		if ((restart_needed_bits & RESTART_UPNP) != 0)
			restart_total_time = MAX(ITVL_RESTART_UPNP, restart_total_time);
		if ((restart_needed_bits & RESTART_DMS) != 0)
			restart_total_time = MAX(ITVL_RESTART_DMS, restart_total_time);
		if ((restart_needed_bits & RESTART_TORRENT) != 0)
			restart_total_time = MAX(ITVL_RESTART_TORRENT, restart_total_time);
		if ((restart_needed_bits & RESTART_ARIA) != 0)
			restart_total_time = MAX(ITVL_RESTART_ARIA, restart_total_time);
		if ((restart_needed_bits & RESTART_ITUNES) != 0)
			restart_total_time = MAX(ITVL_RESTART_ITUNES, restart_total_time);
		if ((restart_needed_bits & RESTART_SWITCH) != 0)
			restart_total_time = MAX(ITVL_RESTART_SWITCH, restart_total_time);
		if ((restart_needed_bits & RESTART_SWITCH_VLAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_SWITCH_VLAN, restart_total_time);
		if ((restart_needed_bits & RESTART_SYSLOG) != 0)
			restart_total_time = MAX(ITVL_RESTART_SYSLOG, restart_total_time);
		if ((restart_needed_bits & RESTART_TWEAKS) != 0)
			restart_total_time = MAX(ITVL_RESTART_TWEAKS, restart_total_time);
		if ((restart_needed_bits & RESTART_FIREWALL) != 0)
			restart_total_time = MAX(ITVL_RESTART_FIREWALL, restart_total_time);
		if ((restart_needed_bits & RESTART_NTPC) != 0)
			restart_total_time = MAX(ITVL_RESTART_NTPC, restart_total_time);
		if ((restart_needed_bits & RESTART_NFS) != 0)
			restart_total_time = MAX(ITVL_RESTART_NFS, restart_total_time);
		if ((restart_needed_bits & RESTART_TIME) != 0)
			restart_total_time = MAX(ITVL_RESTART_TIME, restart_total_time);
		if ((restart_needed_bits & RESTART_SPOOLER) != 0)
			restart_total_time = MAX(ITVL_RESTART_SPOOLER, restart_total_time);
		if ((restart_needed_bits & RESTART_HDDTUNE) != 0)
			restart_total_time = MAX(ITVL_RESTART_HDDTUNE, restart_total_time);
		if ((restart_needed_bits & RESTART_SYSCTL) != 0)
			restart_total_time = MAX(ITVL_RESTART_SYSCTL, restart_total_time);
		if ((restart_needed_bits & RESTART_WIFI) != 0) {
			unsigned int max_time = 1;
			if (wl_modified) {
				if ((wl_modified & WIFI_COMMON_CHANGE_BIT) && nvram_get_int("wl_radio_x"))
					max_time = ITVL_RESTART_WIFI;
				restart_total_time = MAX(max_time, restart_total_time);
			}
			else if (rt_modified) {
				if ((rt_modified & WIFI_COMMON_CHANGE_BIT) && nvram_get_int("rt_radio_x"))
					max_time = ITVL_RESTART_WIFI_INIC;
				restart_total_time = MAX(max_time, restart_total_time);
			}
		}
		
		websWrite(wp, "<script>restart_needed_time(%d);</script>\n", restart_total_time);
	}

	return 0;
}

static int asus_nvram_commit(int eid, webs_t wp, int argc, char **argv) {
	if (nvram_modified) {
		nvram_modified = 0;
		nvram_commit_safe();
	}
	
	return 0;
}

static int ej_notify_services(int eid, webs_t wp, int argc, char **argv) {
	if (restart_needed_bits != 0) {
		if ((restart_needed_bits & RESTART_REBOOT) != 0) {
			if (nvram_get_int("nvram_manual") == 1)
				nvram_commit();
			sys_reboot();
		}
		else {
			dbG("debug restart_needed_bits before: 0x%lx\n", restart_needed_bits);
			
			if ((restart_needed_bits & RESTART_IPV6) != 0) {
				notify_rc("restart_ipv6");
				restart_needed_bits &= ~(u32)RESTART_IPV6;
				restart_needed_bits &= ~(u32)RESTART_LAN;
				restart_needed_bits &= ~(u32)RESTART_DHCPD;		// dnsmasq already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_RADVD;		// radvd already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_DI;		// detect_internet already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_WAN;		// wan already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_MODEM;		// wan already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_IPTV;		// iptv already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_UPNP;		// miniupnpd already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_SWITCH_VLAN;	// vlan filter already re-started (RESTART_IPV6)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_IPV6)
			}
			if ((restart_needed_bits & RESTART_LAN) != 0) {
				notify_rc("restart_whole_lan");
				restart_needed_bits &= ~(u32)RESTART_LAN;
				restart_needed_bits &= ~(u32)RESTART_DHCPD;		// dnsmasq already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_DI;		// detect_internet already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_UPNP;		// miniupnpd already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_VPNSVR;		// vpn server already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_IPTV;		// igmpproxy already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_LAN)
			}
#if (BOARD_NUM_USB_PORTS > 0)
			if ((restart_needed_bits & RESTART_MODEM) != 0) {
				notify_rc("restart_modem");
				restart_needed_bits &= ~(u32)RESTART_MODEM;
				restart_needed_bits &= ~(u32)RESTART_WAN;		// wan already re-started (RESTART_MODEM)
			}
#endif
			if ((restart_needed_bits & RESTART_WAN) != 0) {
				notify_rc("restart_whole_wan");
				restart_needed_bits &= ~(u32)RESTART_WAN;
				restart_needed_bits &= ~(u32)RESTART_IPTV;		// iptv already re-started (RESTART_WAN)
				restart_needed_bits &= ~(u32)RESTART_SWITCH_VLAN;	// vlan filter already re-started (RESTART_WAN)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_WAN)
			}
			if ((restart_needed_bits & RESTART_SWITCH) != 0) {
				notify_rc("restart_switch_config");
				restart_needed_bits &= ~(u32)RESTART_SWITCH;
			}
			if ((restart_needed_bits & RESTART_SWITCH_VLAN) != 0) {
				notify_rc("restart_switch_vlan");
				restart_needed_bits &= ~(u32)RESTART_SWITCH_VLAN;
			}
			if ((restart_needed_bits & RESTART_IPTV) != 0) {
				notify_rc("restart_iptv");
				restart_needed_bits &= ~(u32)RESTART_IPTV;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_IPTV)
			}
			if ((restart_needed_bits & RESTART_RADVD) != 0) {
				notify_rc("restart_radvd");
				restart_needed_bits &= ~(u32)RESTART_RADVD;
				restart_needed_bits &= ~(u32)RESTART_DHCPD;		// dnsmasq already re-started (RESTART_RADVD)
			}
			if ((restart_needed_bits & RESTART_DHCPD) != 0) {
				notify_rc("restart_dhcpd");
				restart_needed_bits &= ~(u32)RESTART_DHCPD;
			}
			if ((restart_needed_bits & RESTART_TERMINAL) != 0) {
				notify_rc("restart_term");
				restart_needed_bits &= ~(u32)RESTART_TERMINAL;
			}
			if ((restart_needed_bits & RESTART_VPNSVR) != 0) {
				notify_rc("restart_vpn_server");
				restart_needed_bits &= ~(u32)RESTART_VPNSVR;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_VPNSVR)
			}
			if ((restart_needed_bits & RESTART_VPNCLI) != 0) {
				notify_rc("restart_vpn_client");
				restart_needed_bits &= ~(u32)RESTART_VPNCLI;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_VPNCLI)
			}
			if ((restart_needed_bits & RESTART_HTTPD) != 0) {
				notify_rc("restart_httpd");
				restart_needed_bits &= ~(u32)RESTART_HTTPD;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_HTTPD)
			}
			if ((restart_needed_bits & RESTART_DI) != 0) {
				notify_rc("restart_di");
				restart_needed_bits &= ~(u32)RESTART_DI;
			}
			if ((restart_needed_bits & RESTART_DDNS) != 0) {
				notify_rc("restart_ddns");
				restart_needed_bits &= ~(u32)RESTART_DDNS;
			}
			if ((restart_needed_bits & RESTART_UPNP) != 0) {
				notify_rc("restart_upnp");
				restart_needed_bits &= ~(u32)RESTART_UPNP;
			}
			if ((restart_needed_bits & RESTART_FIREWALL) != 0) {
				notify_rc("restart_firewall");
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;
			}
#if (BOARD_NUM_USB_PORTS > 0)
			if ((restart_needed_bits & RESTART_FTPSAMBA) != 0) {
				notify_rc("restart_cifs");
				restart_needed_bits &= ~(u32)RESTART_FTPSAMBA;
			}
#if defined(APP_NFSD)
			if ((restart_needed_bits & RESTART_NFS) != 0) {
				notify_rc("restart_nfs");
				restart_needed_bits &= ~(u32)RESTART_NFS;
			}
#endif
#if defined(APP_MINIDLNA)
			if ((restart_needed_bits & RESTART_DMS) != 0) {
				notify_rc("restart_dms");
				restart_needed_bits &= ~(u32)RESTART_DMS;
			}
#endif
#if defined(APP_TRMD)
			if ((restart_needed_bits & RESTART_TORRENT) != 0) {
				notify_rc("restart_torrent");
				restart_needed_bits &= ~(u32)RESTART_TORRENT;
			}
#endif
#if defined(APP_ARIA)
			if ((restart_needed_bits & RESTART_ARIA) != 0) {
				notify_rc("restart_aria");
				restart_needed_bits &= ~(u32)RESTART_ARIA;
			}
#endif
#if defined(APP_FIREFLY)
			if ((restart_needed_bits & RESTART_ITUNES) != 0) {
				notify_rc("restart_itunes");
				restart_needed_bits &= ~(u32)RESTART_ITUNES;
			}
#endif
			if ((restart_needed_bits & RESTART_SPOOLER) != 0) {
				notify_rc("restart_spooler");
				restart_needed_bits &= ~(u32)RESTART_SPOOLER;
			}
			if ((restart_needed_bits & RESTART_HDDTUNE) != 0) {
				notify_rc("restart_hddtune");
				restart_needed_bits &= ~(u32)RESTART_HDDTUNE;
			}
#endif
			if ((restart_needed_bits & RESTART_SYSLOG) != 0) {
				notify_rc("restart_syslog");
				restart_needed_bits &= ~(u32)RESTART_SYSLOG;
			}
			if ((restart_needed_bits & RESTART_TWEAKS) != 0) {
				notify_rc("restart_tweaks");
				restart_needed_bits &= ~(u32)RESTART_TWEAKS;
			}
			if ((restart_needed_bits & RESTART_SYSCTL) != 0) {
				notify_rc("restart_sysctl");
				restart_needed_bits &= ~(u32)RESTART_SYSCTL;
			}
			if ((restart_needed_bits & RESTART_NTPC) != 0) {
				notify_rc("restart_ntpc");
				restart_needed_bits &= ~(u32)RESTART_NTPC;
			}
			if ((restart_needed_bits & RESTART_TIME) != 0) {
				notify_rc("restart_time");
				restart_needed_bits &= ~(u32)RESTART_TIME;
			}
			if ((restart_needed_bits & RESTART_WIFI) != 0) {
#if BOARD_HAS_5G_RADIO
				if (wl_modified) {
					if (wl_modified & WIFI_COMMON_CHANGE_BIT)
						notify_rc("restart_wifi_wl");
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
				if (rt_modified) {
					if (rt_modified & WIFI_COMMON_CHANGE_BIT)
						notify_rc("restart_wifi_rt");
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
				restart_needed_bits &= ~(u32)RESTART_WIFI;
			}
			
			dbG("debug restart_needed_bits after: 0x%lx\n", restart_needed_bits);
		}
		restart_needed_bits = 0;
	}

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

static int
wanlink_hook(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char wan_dns[512], wan_mac[18], etherlink[32], apclilink[32];
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
		
#if (BOARD_NUM_USB_PORTS > 0)
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
				} else {
					strcpy(apclilink, "---");
					phy_failed = 2; // STA not connected
				}
				
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
				if (wan_ifstate > 0 && (wan_proto != IPV4_WAN_PROTO_PPPOE || nvram_get_int("hw_nat_mode") == 2)) {
					wan_bytes_rx = get_ifstats_bytes_rx(wan_ifname);
					wan_bytes_tx = get_ifstats_bytes_tx(wan_ifname);
				}
				
				ppp_mode = (wan_proto == IPV4_WAN_PROTO_L2TP) ? 2 : 1;
			} else {
				wan_ifstate = get_if_state(man_ifname, addr4_wan);
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
					char *l2tpd = (nvram_match("wan_l2tpd", "0")) ? "xl2tpd" : "l2tpd";
					status_code = (pids(l2tpd)) ? INET_STATE_PPP_WAIT : INET_STATE_PPP_INACTIVE;
				} else
					status_code = (pids("pppd")) ? INET_STATE_PPP_WAIT : INET_STATE_PPP_INACTIVE;
			} else
				status_code = (wan_ifstate == IF_STATE_UP) ? INET_STATE_NETIF_WAIT_DHCP : INET_STATE_NETIF_NOT_READY;
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
		char dns_item[80] = {0};
		while(fscanf(fp, "nameserver %s\n", dns_item) > 0) {
			if (*dns_item && strcmp(dns_item, "127.0.0.1") != 0) {
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
	char etherlink0[32] = {0};
	char etherlink1[32] = {0};
	char etherlink2[32] = {0};
	char etherlink3[32] = {0};
	char etherlink4[32] = {0};

	fill_eth_port_status(0, etherlink0);
	fill_eth_port_status(1, etherlink1);
	fill_eth_port_status(2, etherlink2);
	fill_eth_port_status(3, etherlink3);
	fill_eth_port_status(4, etherlink4);

	websWrite(wp, "function lanlink_etherlink_wan()  { return '%s';}\n", etherlink0);
	websWrite(wp, "function lanlink_etherlink_lan1() { return '%s';}\n", etherlink1);
	websWrite(wp, "function lanlink_etherlink_lan2() { return '%s';}\n", etherlink2);
	websWrite(wp, "function lanlink_etherlink_lan3() { return '%s';}\n", etherlink3);
	websWrite(wp, "function lanlink_etherlink_lan4() { return '%s';}\n", etherlink4);

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
#if (BOARD_NUM_USB_PORTS > 0)
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
nvram_action_hook(int eid, webs_t wp, int argc, char **argv) 
{
	int commit_all = 0;
	int sys_result = 0;
	char *action_id = websGetVar(wp, "nvram_action", "");

	if (strcmp(action_id, "commit_all") == 0)
		commit_all = 1;

	if (commit_all || strcmp(action_id, "commit_nvram") == 0)
	{
		sys_result |= nvram_commit();
	}

	if (commit_all || strcmp(action_id, "commit_storage") == 0)
	{
		sys_result |= doSystem("/sbin/mtd_storage.sh %s", "save");
	}

	websWrite(wp, "{\"nvram_result\": %d}", sys_result);

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
#if defined(APP_FTPD)
	int found_app_ftpd = 1;
#else
	int found_app_ftpd = 0;
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
#if defined(USE_IPV6_HW_NAT)
#if defined(USE_HW_NAT_V2)
	int has_ipv6_ppe = 2;
#else
	int has_ipv6_ppe = 1;
#endif
#else
	int has_ipv6_ppe = 0;
#endif
#if defined(USE_RT3352_MII)
	int min_vlan_ext = 4;
#else
	int min_vlan_ext = 3;
#endif
#if (BOARD_RAM_SIZE < 64)
	int max_conn = 16384;
#elif (BOARD_RAM_SIZE < 128)
	int max_conn = 65536;
#elif (BOARD_RAM_SIZE < 256)
	int max_conn = 262144;
#else
	int max_conn = 524288;
#endif
#if (BOARD_NUM_USB_PORTS > 0)
	int has_usb = 1;
#else
	int has_usb = 0;
#endif
#if defined(USE_USB3)
	int has_usb3 = 1;
#else
	int has_usb3 = 0;
#endif
#if defined(SUPPORT_PEAP_SSL)
	int has_peap_ssl = 1;
#else
	int has_peap_ssl = 0;
#endif
#if defined(SUPPORT_HTTPS)
	int has_http_ssl = 1;
#else
	int has_http_ssl = 0;
#endif
#if defined(SUPPORT_DDNS_SSL)
	int has_ddns_ssl = 1;
#else
	int has_ddns_ssl = 0;
#endif
#if defined(USE_RT3352_MII)
	int has_inic_mii = 1;
#else
	int has_inic_mii = 0;
#endif
#if defined(USE_RTL8367)
	int use_switch_type = 0; // Realtek RTL8367
#elif defined(USE_MTK_ESW)
	int use_switch_type = 1; // Mediatek MT7620 Embedded ESW
#elif defined(USE_MTK_GSW)
	int use_switch_type = 2; // Mediatek MT7621 Internal GSW (or MT7530)
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
		"function found_app_ftpd() { return %d;}\n"
		"function found_srv_u2ec() { return %d;}\n"
		"function found_srv_lprd() { return %d;}\n"
		"function found_app_sshd() { return %d;}\n"
		"function found_app_xupnpd() { return %d;}\n",
		found_utl_hdparm,
		found_app_ovpn,
		found_app_dlna,
		found_app_ffly,
		found_app_trmd,
		found_app_aria,
		found_app_nfsd,
		found_app_smbd,
		found_app_ftpd,
		found_srv_u2ec,
		found_srv_lprd,
		found_app_sshd,
		found_app_xupnpd
	);

	websWrite(wp,
		"function support_ipv6() { return %d;}\n"
		"function support_ipv6_ppe() { return %d;}\n"
		"function support_peap_ssl() { return %d;}\n"
		"function support_http_ssl() { return %d;}\n"
		"function support_ddns_ssl() { return %d;}\n"
		"function support_min_vlan() { return %d;}\n"
		"function support_max_conn() { return %d;}\n"
		"function support_usb() { return %d;}\n"
		"function support_usb3() { return %d;}\n"
		"function support_switch_type() { return %d;}\n"
		"function support_2g_apcli_only() { return %d;}\n"
		"function support_5g_radio() { return %d;}\n"
		"function support_5g_11ac() { return %d;}\n"
		"function support_5g_stream_tx() { return %d;}\n"
		"function support_5g_stream_rx() { return %d;}\n"
		"function support_2g_stream_tx() { return %d;}\n"
		"function support_2g_stream_rx() { return %d;}\n",
		has_ipv6,
		has_ipv6_ppe,
		has_peap_ssl,
		has_http_ssl,
		has_ddns_ssl,
		min_vlan_ext,
		max_conn,
		has_usb,
		has_usb3,
		use_switch_type,
		(has_inic_mii) ? 0 : 1,
		BOARD_HAS_5G_RADIO,
		BOARD_HAS_5G_11AC,
		BOARD_NUM_ANT_5G_TX,
		BOARD_NUM_ANT_5G_RX,
		BOARD_NUM_ANT_2G_TX,
		BOARD_NUM_ANT_2G_RX
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
#if defined (BOARD_GPIO_BTN_WLTOG)
	int has_but_wlt = 1;
#else
	int has_but_wlt = 0;
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
#if defined (BOARD_GPIO_LED_USB) && (BOARD_NUM_USB_PORTS > 0)
	int has_led_usb = 1;
#else
	int has_led_usb = 0;
#endif
#if defined (BOARD_GPIO_LED_WIFI)
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
		"function support_but_wlt() { return %d;}\n"
		"function support_led_all() { return %d;}\n"
		"function support_led_wan() { return %d;}\n"
		"function support_led_lan() { return %d;}\n"
		"function support_led_usb() { return %d;}\n"
		"function support_led_wif() { return %d;}\n"
		"function support_led_pwr() { return %d;}\n"
		"function support_led_phy() { return %d;}\n",
		has_but_wps,
		has_but_wlt,
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
	bool last_was_escaped;
	int ret = 0;
	
	if (argc < 1) {
		websError(wp, 400,
				"get_parameter() used with no arguments, but at least one "
				"argument is required to specify the parameter name\n");
		return -1;
	}
	
	last_was_escaped = FALSE;
	
	char *value = websGetVar(wp, argv[0], "");
	websWrite(wp, "%s", value);//*/
	return ret;
}

static int login_state_hook(int eid, webs_t wp, int argc, char **argv) {
#if defined(USE_IPV6)
	char s_addr[INET6_ADDRSTRLEN];
#else
	char s_addr[INET_ADDRSTRLEN];
#endif
	fill_login_ip(s_addr, sizeof(s_addr));

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
	websWrite(wp, "function board_flash_time() { return %d;}\n", BOARD_FLASH_TIME);

	return 0;
}

static int ej_get_vpns_client(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	int first_client;
	char ifname[16], addr_l[32], addr_r[32], peer_name[64];
	
	fp = fopen("/tmp/vpns.leases", "r");
	if (!fp) {
		return 0;
	}
	
	first_client = 1;
	while (fscanf(fp, "%s %s %s %[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4) {
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
	char buf[32];
	char line_buf[64];

	fp = fopen("/proc/meminfo", "r");
	if (fp)
	{
		if (fgets(line_buf, sizeof(line_buf), fp) && 
		    sscanf(line_buf, "MemTotal: %lu %s", &st->total, buf) == 2)
		{
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "MemFree: %lu %s", &st->free, buf);
			
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "Buffers: %lu %s", &st->buffers, buf);
			
			fgets(line_buf, sizeof(line_buf), fp);
			sscanf(line_buf, "Cached: %lu %s", &st->cached, buf);
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
		st->radio = (nvram_get_int("mlme_radio_rt")) ? 1 : 0;
		if (st->radio)
			st->ap_guest = is_interface_up(IFNAME_2G_GUEST);
		else
			st->ap_guest = 0;
	}
}


#define LOAD_INT(x)	(unsigned)((x) >> 16)
#define LOAD_FRAC(x)	LOAD_INT(((x) & ((1 << 16) - 1)) * 100)

static struct cpu_stats g_cpu_old = {0};

static int ej_system_status_hook(int eid, webs_t wp, int argc, char **argv)
{
	struct sysinfo info;
	struct cpu_stats cpu;
	struct mem_stats mem;
	struct wifi_stats wifi2;
	struct wifi_stats wifi5;
	struct stat log;
	unsigned long updays, uphours, upminutes;
	unsigned diff_total, u_user, u_nice, u_system, u_idle, u_iowait, u_irq, u_sirq, u_busy;

	if (g_cpu_old.total == 0)
	{
		get_cpudata(&g_cpu_old);
		usleep(100000);
	}

	get_cpudata(&cpu);
	get_memdata(&mem);
	get_wifidata(&wifi2, 0);
	get_wifidata(&wifi5, 1);

	diff_total = (unsigned)(cpu.total - g_cpu_old.total);
	if (!diff_total) diff_total = 1;

	u_user = (unsigned)(cpu.user - g_cpu_old.user) * 100 / diff_total;
	u_nice = (unsigned)(cpu.nice - g_cpu_old.nice) * 100 / diff_total;
	u_system = (unsigned)(cpu.system - g_cpu_old.system) * 100 / diff_total;
	u_idle = (unsigned)(cpu.idle - g_cpu_old.idle) * 100 / diff_total;
	u_iowait = (unsigned)(cpu.iowait - g_cpu_old.iowait) * 100 / diff_total;
	u_irq = (unsigned)(cpu.irq - g_cpu_old.irq) * 100 / diff_total;
	u_sirq = (unsigned)(cpu.sirq - g_cpu_old.sirq) * 100 / diff_total;
	u_busy = (unsigned)(cpu.busy - g_cpu_old.busy) * 100 / diff_total;

	memcpy(&g_cpu_old, &cpu, sizeof(struct cpu_stats));

	sysinfo(&info);
	updays = (unsigned long) info.uptime / (unsigned long)(60*60*24);
	upminutes = (unsigned long) info.uptime / (unsigned long)60;
	uphours = (upminutes / (unsigned long)60) % (unsigned long)24;
	upminutes %= 60;

	mem.sw_total = (unsigned long)(((unsigned long long)info.totalswap * info.mem_unit) >> 10);
	mem.sw_free  = (unsigned long)(((unsigned long long)info.freeswap * info.mem_unit) >> 10);

	if (stat("/tmp/syslog.log", &log) != 0)
		log.st_mtime = 0;

	websWrite(wp, "{\"lavg\": \"%u.%02u %u.%02u %u.%02u\", "
			"\"uptime\": {\"days\": %lu, \"hours\": %lu, \"minutes\": %lu}, "
			"\"ram\": {\"total\": %lu, \"used\": %lu, \"free\": %lu, \"buffers\": %lu, \"cached\": %lu}, "
			"\"swap\": {\"total\": %lu, \"used\": %lu, \"free\": %lu}, "
			"\"cpu\": {\"busy\": %u, \"user\": %u, \"nice\": %u, \"system\": %u, "
				  "\"idle\": %u, \"iowait\": %u, \"irq\": %u, \"sirq\": %u}, "
			"\"wifi2\": {\"state\": %d, \"guest\": %d}, "
			"\"wifi5\": {\"state\": %d, \"guest\": %d}, "
			"\"logmt\": %ld }",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]),
			updays, uphours, upminutes,
			mem.total, (mem.total - mem.free), mem.free, mem.buffers, mem.cached,
			mem.sw_total, (mem.sw_total - mem.sw_free), mem.sw_free,
			u_busy, u_user, u_nice, u_system, u_idle, u_iowait, u_irq, u_sirq,
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
				websWrite(wp, buf);
				log_lines++;
			}
			
			fclose(fp);
		}
	}

	if (!log_lines)
		websWrite(wp, "%s", "");

	return 0;
}

#define MAX_DICT_LANGS (15)
int ej_shown_language_option(int eid, webs_t wp, int argc, char **argv) {
	FILE *fp;
	int i, len;
	struct language_table *pLang = NULL;
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
apply_cgi(webs_t wp, char *urlPrefix, char *webDir, int arg,
		char *url, char *path, char *query)
{
	int sid;
	char *value;
	char *current_url;
	char *next_url;
	char *sid_list;
	char *value1;
	char *script;
	char groupId[64];
	char urlStr[64];
	char *groupName;
	char *syscmd;

	urlStr[0] = 0;

	value = websGetVar(wp, "action_mode","");
	next_host = websGetVar(wp, "next_host", "");
	current_url = websGetVar(wp, "current_page", "");
	next_url = websGetVar(wp, "next_page", "");
	script = websGetVar(wp, "action_script","");
	
	if (!strcmp(value, " Refresh "))
	{
		syscmd = websGetVar(wp, "SystemCmd", "");
		strncpy(SystemCmd, syscmd, sizeof(SystemCmd)-1);
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value, " Clear "))
	{
		// current only syslog implement this button
		unlink("/tmp/syslog.log");
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value, " Restart "))
	{
		websApply(wp, "Restarting.asp");
		sys_reboot();
		return (0);
	}
	else if (!strcmp(value, " RestoreNVRAM "))
	{
		websApply(wp, "Restarting.asp");
		eval("/sbin/reset_to_defaults");
		return (0);
	}
	else if (!strcmp(value, " RestoreStorage "))
	{
		websApply(wp, "Restarting.asp");
		doSystem("/sbin/mtd_storage.sh %s", "erase");
		sys_reboot();
		return (0);
	}
	else
	{
		sid_list = websGetVar(wp, "sid_list", "");
		while ((serviceId = svc_pop_list(sid_list, ';')))
		{
			sid = 0;
			while (GetServiceId(sid) != 0)
			{
				if (!strcmp(GetServiceId(sid), serviceId))
					break;
				
				++sid;
			}
			
			if (serviceId != NULL)
			{
				if (!strcmp(value, " Restore "))
				{
					;
				}
				else if (!strcmp(value, "  Save  ") || !strcmp(value, " Apply "))
				{
					validate_cgi(wp, sid);
				}
				else if (!strcmp(value, "Set") || !strcmp(value, "Unset") || 
					 !strcmp(value, "Update") || !strcmp(value, "Eject") || !strcmp(value, "Check") )
				{
					validate_cgi(wp, sid);
				}
				else
				{
					strcpy(groupId,websGetVar(wp, "group_id", ""));
					
					if ((value1 = websGetVar(wp, "action_mode", NULL)) != NULL)
					{
						groupName = groupId;
						
						//if (!strcmp(GetServiceId(sid), groupId))
						{
							if (!strncmp(value1, " Delete ", 8))
							{
								apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_DELETE);
							}
							else if (!strncmp(value1, " Add ", 5))
							{
								apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_ADD);
							}
							else if (!strncmp(value1, " Del ", 5))
							{
								apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_REMOVE);
							}
							else if (!strncmp(value1, " Refresh ", 9))
							{
								apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_REFRESH);
							}
							
							snprintf(urlStr, sizeof(urlStr), "%s#%s", current_url, groupName);
							validate_cgi(wp, sid);
						}
					}
				}
			}
			
			sid_list = sid_list+strlen(serviceId)+1;
		}
		
		/* Add for EMI Test page */
		if (strcmp(script, ""))
		{
			sys_script(script);
		}
		
		if (!strcmp(value, "  Save  ") || !strcmp(value, " Apply "))
			websRedirect(wp, next_url);
		else if (urlStr[0] == 0)
			websRedirect(wp, current_url);
		else
			websRedirect(wp, urlStr);
		
		return 0;
	}
	
	return 1;
}

void nvram_add_group_item(webs_t wp, struct variable *v, int sid)
{
    struct variable *gv;
    char *value;
    char name[64];
    int count;

    if (v->argv[0]==NULL) 
    {
       return;
    }

    count = nvram_get_int(v->argv[3]);

    for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++)
    {
	sprintf(name, "%s_0", gv->name);
	if ((value=websGetVar(wp, name, NULL)))
	{
	    nvram_add_list_x(gv->name, value, count);
	}
	else
	{
	    nvram_add_list_x(gv->name, "", count);
	}
    }

    count++;
    nvram_set_int(v->argv[3], count);
}


void nvram_remove_group_item(webs_t wp, struct variable *v, int sid, int *delMap)
{
    struct variable *gv;
    int i, deleted, gcount;

    if (v->argv[0]==NULL) 
    {
       return;
    }

    gcount = nvram_get_int(v->argv[3]);
    if (gcount < 1)
    {
       return;
    }

    for (gv = (struct variable *)v->argv[0]; gv->name!=NULL; gv++)
    {
        nvram_del_list_map_x(gv->name, gcount, delMap);
    }

    deleted = 0;
    for (i=0; i < gcount; i++)
    {
        if (delMap[deleted]==i)
            deleted++;
    }

    if (deleted > 0)
    {
        gcount -= deleted;
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
    	   sprintf(bufs,"%s", buf);
    	else
    	   sprintf(bufs,"%s%s",bufs, buf);
    	
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
apply_cgi_group(webs_t wp, int sid, struct variable *var, char *groupName, int flag)
{
   struct variable *v;
   int groupCount;

   if (var!=NULL) v = var;
   else
   {
       /* Validate and set vairables in table order */
       for (v = GetVariables(sid); v->name != NULL; v++)
       {
       	   if (!strcmp(groupName, v->name)) 
       		break;
       }
   }

   if (v->name == NULL) return 0;

   groupCount = atoi(v->argv[1]);

   if (flag == GROUP_FLAG_ADD)/* if (!strcmp(value, " Refresh ")) || Save condition */
   {
	nvram_add_group_item(wp, v, sid);
		return 1;	// 2008.08 magic
   }
   else if (flag == GROUP_FLAG_REMOVE)/* if (!strcmp(value, " Refresh ")) || Save condition */
   {
   	nvram_remove_group_item(wp, v, sid, delMap);  
		return 1; 	// 2008.08 magic
   }
  	return 0; // 2008.08 magic
}

static int
nvram_generate_table(webs_t wp, char *serviceId, char *groupName)
{
   struct variable *v;
   int i, groupCount, ret, r, sid;

   sid = LookupServiceId(serviceId);

   if (sid==-1) return 0;

   /* Validate and set vairables in table order */
   for (v = GetVariables(sid); v->name != NULL; v++) 
   {
      if (!strcmp(groupName, v->name)) break;
   }

   if (v->name == NULL) return 0;    

   groupCount = nvram_get_int(v->argv[3]);

   if (groupCount==0)
       ret = nvram_add_group_table(wp, serviceId, v, -1);
   else
   {
      ret = 0;
      for (i=0; i<groupCount; i++)
      {
	r = nvram_add_group_table(wp, serviceId, v, i);
	if (r!=0)
	   ret += r;
	else break;
      }
   }

   return (ret);
}


static void
do_apply_cgi(char *url, FILE *stream)
{
	apply_cgi(stream, NULL, NULL, 0, url, NULL, NULL);
}

#define SWAP_LONG(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))

static int
check_nvram_header(char *buf, long *filelen)
{
	long *filelenptr;

	if (strncmp(buf, PROFILE_HEADER, 4) && strncmp(buf, PROFILE_HEADER_NEW, 4)) {
		httpd_log("%s: Incorrect NVRAM profile header!", "NVRAM restore");
		return -1;
	}

	filelenptr = (long*)(buf + 4);
	*filelen = *filelenptr & 0xffffff;

	return 0;
}

static int
check_image_header(char *buf, long *filelen)
{
	int pid_asus_len;
	char pid_asus[16];
	image_header_t *hdr = (image_header_t *)buf;

	/* check header magic */
	if (SWAP_LONG(hdr->ih_magic) != IH_MAGIC) {
		httpd_log("%s: Incorrect image header!", "Firmware update");
		return -1;
	}

	pid_asus_len = strlen(BOARD_PID);
	if (pid_asus_len > 12)
		pid_asus_len = 12;

	strncpy(pid_asus, buf+36, pid_asus_len);
	pid_asus[pid_asus_len] = 0;

	if (strcmp(pid_asus, BOARD_PID) != 0) {
		httpd_log("%s: Incorrect image ProductID: %s! Expected is %s.", "Firmware update", pid_asus, BOARD_PID);
		return -2;
	}

	*filelen = SWAP_LONG(hdr->ih_size) + sizeof(image_header_t);

	return 0;
}

static int
check_image_crc(const char *fw_image)
{
	int ifd;
	uint32_t checksum, datalen;
	struct stat sbuf;
	unsigned char *ptr = (unsigned char *)MAP_FAILED;
	image_header_t header2;
	image_header_t *hdr, *hdr2=&header2;
	int ret=0;

	ifd = open(fw_image, O_RDONLY|O_BINARY);

	if (ifd < 0) {
		ret=-1;
		goto checkcrc_end;
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif
	if (fstat(ifd, &sbuf) < 0) {
		ret=-1;
		goto checkcrc_fail;
	}

	if ((unsigned int)sbuf.st_size < (sizeof(image_header_t) + (2 * 1024 * 1024)) || 
	    (unsigned int)sbuf.st_size > get_mtd_size(FW_MTD_NAME)) {
		httpd_log("%s: Firmware image size is invalid!", "Firmware update");
		ret=-1;
		goto checkcrc_fail;
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		ret=-1;
		goto checkcrc_fail;
	}

	hdr = (image_header_t *)ptr;

	memcpy(hdr2, hdr, sizeof(image_header_t));
	memset(&hdr2->ih_hcrc, 0, sizeof(uint32_t));
	checksum = crc32_sp(0, (const char *)hdr2, sizeof(image_header_t));

	if (checksum!=SWAP_LONG(hdr->ih_hcrc))
	{
		ret=-1;
		httpd_log("%s: Firmware image %s has invalid CRC!", "Firmware update", "header");
		goto checkcrc_fail;
	}

	datalen = SWAP_LONG(hdr->ih_size);
	if (datalen > ((unsigned int)sbuf.st_size - sizeof(image_header_t)))
	{
		ret=-1;
		httpd_log("%s: Firmware image is corrupted! Please check free space in /tmp!", "Firmware update");
		goto checkcrc_fail;
	}

	checksum = crc32_sp(0, (const char *)(ptr + sizeof(image_header_t)), datalen);

	if (checksum!=SWAP_LONG(hdr->ih_dcrc))
	{
		ret=-1;
		httpd_log("%s: Firmware image %s has invalid CRC!", "Firmware update", "body");
		goto checkcrc_fail;
	}

checkcrc_fail:
	if (ptr != (unsigned char *)MAP_FAILED)
		munmap((void *)ptr, sbuf.st_size);

#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif
	if (close(ifd)) {
		fprintf (stderr, "Read error on %s: %s\n", fw_image, strerror(errno));
		ret=-1;
	}
checkcrc_end:
	return ret;
}

static int chk_image_err = 1;
static int chk_nvram_err = 1;

static void
do_upgrade_post(char *url, FILE *stream, int clen, char *boundary)
{
	FILE *fifo = NULL;
	char upload_fifo[] = FW_IMG_NAME;
	char buf[4096];
	int cnt, count, offset, ret, ch;
	long filelen;
	int valid_header = 0;

	ret = EINVAL;
	chk_image_err = 1;

	// delete some files (need free space in /tmp)
	unlink("/tmp/usb.log");
	unlink("/tmp/syscmd.log");
	doSystem("rm -rf %s", "/tmp/xupnpd-cache");
	doSystem("rm -rf %s", "/tmp/xupnpd-feeds");

	/* Look for our part */
	while (clen > 0)
	{
		if (!fgets(buf, MIN(clen + 1, sizeof(buf)), stream))
			goto err;
		clen -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20) && strstr(buf, "name=\"file\""))
			break;
	}

	/* Skip boundary and headers */
	while (clen > 0) {
		if (!fgets(buf, MIN(clen + 1, sizeof(buf)), stream))
			goto err;
		clen -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	fput_int("/proc/sys/vm/drop_caches", 1);

	/* copy mtd_write to RAM */
	doSystem("cp -f %s %s", "/bin/mtd_write", "/tmp");

	if (!(fifo = fopen(upload_fifo, "w+"))) goto err;

	filelen = clen;
	offset = 0;
	cnt = 0;

	/* Pipe the rest to the FIFO */
	while (clen > 0 && filelen > 0)
	{
		count = fread(buf + offset, 1, MIN(clen, sizeof(buf)-offset), stream);
		if(count <= 0)
			goto err;
		
		clen -= count;
		
		if (cnt == 0)
		{
			if (count + offset < sizeof(image_header_t))
			{
				offset += count;
				continue;
			}
			
			count += offset;
			offset = 0;
			cnt++;
			
			ret = check_image_header(buf, &filelen);
			if (ret != 0)
				goto err;
			
			valid_header=1;
		}
		
		filelen -= count;
		fwrite(buf, 1, count, fifo);
	}

	if (!valid_header)
		goto err;

	/* Slurp anything remaining in the request */
	while (clen-- > 0)
	{
		if((ch = fgetc(stream)) == EOF)
			break;
		
		if (filelen > 0) {
			fwrite(&ch, 1, 1, fifo);
			filelen--;
		}
	}

	fclose(fifo);
	fifo = NULL;

	ret = check_image_crc(upload_fifo);

err:
	if (fifo)
		fclose(fifo);

	/* Slurp anything remaining in the request */
	while (clen-- > 0)
		if((ch = fgetc(stream)) == EOF)
			break;

	if ((ret == 0) && (valid_header))
		chk_image_err = 0;
}

static void
do_upgrade_cgi(char *url, FILE *stream)
{
	if (chk_image_err == 0) {
		notify_rc("flash_firmware");
		websApply(stream, "Updating.asp");
	} else {
		unlink(FW_IMG_NAME);
		websApply(stream, "UpdateError.asp");
	}
}

static void
do_upload_post(char *url, FILE *stream, int clen, char *boundary)
{
	FILE *fifo = NULL;
	char upload_fifo[] = PROFILE_FIFO_UPLOAD;
	char buf[1024];
	int cnt, count, offset, ret, ch;
	long filelen;
	char valid_header = 0;

	ret = EINVAL;
	chk_nvram_err = 1;

	/* Look for our part */
	while (clen > 0) {
		if (!fgets(buf, MIN(clen + 1, sizeof(buf)), stream))
			goto err;
		clen -= strlen(buf);
		if (!strncasecmp(buf, "Content-Disposition:", 20) && strstr(buf, "name=\"file\""))
			break;
	}

	/* Skip boundary and headers */
	while (clen > 0) {
		if (!fgets(buf, MIN(clen + 1, sizeof(buf)), stream))
			goto err;
		clen -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
			break;
	}

	if (!(fifo = fopen(upload_fifo, "w+")))
		goto err;

	filelen = clen;
	offset = 0;
	cnt = 0;

	/* Pipe the rest to the FIFO */
	while (clen > 0 && filelen > 0)
	{
		count = fread(buf + offset, 1, MIN(clen, sizeof(buf)-offset), stream);
		if (count <= 0)
			goto err;
		
		clen -= count;
		
		if (cnt == 0)
		{
			if (count + offset < 8)
			{
				offset += count;
				continue;
			}
			
			count += offset;
			offset = 0;
			cnt++;
			
			ret = check_nvram_header(buf, &filelen);
			if (ret != 0)
				goto err;
			
			valid_header = 1;
		}
		
		filelen -= count;
		fwrite(buf, 1, count, fifo);
	}

	if (!valid_header)
		goto err;

	/* Slurp anything remaining in the request */
	while (clen-- > 0)
	{
		if((ch = fgetc(stream)) == EOF)
			break;
		
		if (filelen > 0) {
			fwrite(&ch, 1, 1, fifo);
			filelen--;
		}
	}

	ret = 0;

	fclose(fifo);
	fifo = NULL;

err:
	if (fifo)
		fclose(fifo);

	/* Slurp anything remaining in the request */
	while (clen-- > 0)
		if((ch = fgetc(stream)) == EOF)
			break;

	if ((ret == 0) && (valid_header))
		chk_nvram_err = 0;
}

static void
do_upload_cgi(char *url, FILE *stream)
{
	/* Reboot if successful */
	if (chk_nvram_err == 0)
	{
		doSystem("killall %s %s", "-q", "watchdog");
		sleep(1);
		websApply(stream, "Uploading.asp");
		eval("nvram", "restore", PROFILE_FIFO_UPLOAD);
		nvram_commit();
		sys_reboot();
	}
	else
	{
		unlink(PROFILE_FIFO_UPLOAD);
		websApply(stream, "UploadError.asp");
	}
}

static void
do_update_cgi(char *url, FILE *stream)
{
	struct ej_handler *handler;
	const char *pattern;
	int argc;
	char *argv[16];
	char s[32];

	if ((pattern = get_cgi("output")) != NULL) {
		for (handler = &ej_handlers[0]; handler->pattern; handler++) {
			if (strcmp(handler->pattern, pattern) == 0) {
				for (argc = 0; argc < 16; ++argc) {
					sprintf(s, "arg%d", argc);
					if ((argv[argc] = (char *)get_cgi(s)) == NULL)
						break;
				}
				handler->output(0, stream, argc, argv);
				break;
			}
		}
	}
}

static void
do_nvram_file(char *url, FILE *stream)
{
	char *nvram_file = PROFILE_FIFO_DOWNLOAD;

	unlink(nvram_file);
	eval("nvram", "save", nvram_file);
	do_file(nvram_file, stream);
}

static void
do_storage_file(char *url, FILE *stream)
{
	char *storage_file = "/tmp/.storage_tar.bz2";

	unlink(storage_file);
	doSystem("/sbin/mtd_storage.sh %s", "backup");
	do_file(storage_file, stream);
}

static void 
do_log_cgi(char *url, FILE *stream)
{
	dump_file(stream, "/tmp/syslog.log");
	fputs("\r\n", stream); /* terminator */
}

static char syslog_txt[] =
"Content-Disposition: attachment;\r\n"
"filename=syslog.txt"
;

static char cache_static[] =
"Cache-Control: max-age=2592000\r\n"
"Expires: Tue, 31 Dec 2014 01:00:00 GMT"
;

static char no_cache_IE[] =
"X-UA-Compatible: IE=edge\r\n"
"Cache-Control: no-store, no-cache, must-revalidate\r\n"
"Pragma: no-cache\r\n"
"Expires: -1"
;

struct mime_handler mime_handlers[] = {
	{ "Nologin.asp", "text/html", no_cache_IE, do_html_post_and_get, do_ej, NULL },
	{ "jquery.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**bootstrap.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**engage.itoggle.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**highstock.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23

	{ "**formcontrol.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "ajax.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "alttxt.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "modem_isp.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "client_function.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "disk_functions.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "md5.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmcal.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmhist.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmmenu.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23

	{ "httpd_check.htm", "text/html", no_cache_IE, do_html_post_and_get, do_ej, NULL },
	{ "**.htm*", "text/html", no_cache_IE, do_html_post_and_get, do_ej, do_auth },
	{ "**.asp*", "text/html", no_cache_IE, do_html_post_and_get, do_ej, do_auth },

	{ "**.css", "text/css", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.png", "image/png", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.gif", "image/gif", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.jpg", "image/jpeg", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.ico", "image/x-icon", cache_static, NULL, do_file, NULL }, // 2013.04 Eagle23

	{ "**.svg", "image/svg+xml", NULL, NULL, do_file, NULL },
	{ "**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL  },
	{ "**.htc", "text/x-component", NULL, NULL, do_file, NULL  },

	{ "general.js",  "text/javascript", no_cache_IE, NULL, do_ej, do_auth },

	{ "**.js",  "text/javascript", no_cache_IE, NULL, do_ej, do_auth },
	{ "**.cab", "text/txt", NULL, NULL, do_file, do_auth },

	{ "Settings_**.CFG", "application/force-download", NULL, NULL, do_nvram_file, do_auth },
	{ "Storage_**.TBZ", "application/force-download", NULL, NULL, do_storage_file, do_auth },
	{ "syslog.txt", "application/force-download", syslog_txt, do_html_post_and_get, do_log_cgi, do_auth },

	{ "apply.cgi*", "text/html", no_cache_IE, do_html_post_and_get, do_apply_cgi, do_auth },
	{ "upgrade.cgi*", "text/html", no_cache_IE, do_upgrade_post, do_upgrade_cgi, do_auth},
	{ "upload.cgi*", "text/html", no_cache_IE, do_upload_post, do_upload_cgi, do_auth },
	{ "update.cgi*", "text/javascript", no_cache_IE, do_html_post_and_get, do_update_cgi, do_auth }, // jerry5 

	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

// traffic monitor
static int 
ej_netdev(int eid, webs_t wp, int argc, char **argv)
{
	FILE *fp;
	char buf[256];
	uint64_t rx, tx;
	char *p, *ifname;
	const char *ifdesc;
	char comma;
	int ret = 0;

	ret += websWrite(wp, "\nnetdev = {\n");
	if ((fp = fopen("/proc/net/dev", "r")) != NULL) {
		fgets(buf, sizeof(buf), fp);
		fgets(buf, sizeof(buf), fp);
		comma = ' ';
		while (fgets(buf, sizeof(buf), fp)) {
			if ((p = strchr(buf, ':')) == NULL) continue;
			*p = 0;
			if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
			else ++ifname;
			
			ifdesc = get_ifname_descriptor(ifname);
			if (!ifdesc)
				continue;
			
			if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &rx, &tx) != 2) continue;
			ret += websWrite(wp, "%c'%s':{rx:0x%llx,tx:0x%llx}", comma, ifdesc, rx, tx);
			comma = ',';
			ret += websWrite(wp, "\n");
		}
		fclose(fp);
		ret += websWrite(wp, "};");
	}
	return 0;
}

static int 
ej_bandwidth(int eid, webs_t wp, int argc, char **argv)
{
	char *name, *sigs;
	int is_speed = 0;

	if (strcmp(argv[0], "speed") == 0) {
		sigs = "-SIGUSR1";
		name = "/var/spool/rstats-speed.js";
		is_speed = 1;
	} else {
		sigs = "-SIGUSR2";
		name = "/var/spool/rstats-history.js";
	}

	if (pids("rstats")) {
		unlink(name);
		doSystem("killall %s %s", sigs, "rstats");
		f_wait_exists(name, 5);
		do_f(name, wp);
	} else {
		if (f_exists(name)) {
			do_f(name, wp);
		} else {
			if (is_speed)
				websWrite(wp, "\nspeed_history = {};\n");
			else
				websWrite(wp, "\ndaily_history = [];\nmonthly_history = [];\n");
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

	if ((argc != 1) || ((list = strdup(argv[0])) == NULL)) return 0;
	websWrite(wp, "\nnvram = {\n");
	p = list;
	while ((k = strsep(&p, ",")) != NULL) {
		if (*k == 0) continue;
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

#if (!BOARD_NUM_USB_PORTS)
static int
ej_get_usb_ports_info(int eid, webs_t wp, int argc, char **argv)
{
	/* usb ports num */
	websWrite(wp, "function get_usb_ports_num(){\n");
	websWrite(wp, "    return %u;\n", 0);
	websWrite(wp, "}\n\n");

	/* usb device types */
	websWrite(wp, "function get_device_type_usb(port_num){\n");
	websWrite(wp, "    return \"%s\";\n", "unknown");
	websWrite(wp, "}\n\n");

	/* printers */
	websWrite(wp, "function printer_ports() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_manufacts() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_models() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	/* modems */
	websWrite(wp, "function modem_ports() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_devnum() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_types() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_manufacts() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_models() {\n");
	websWrite(wp, "    return [");
	websWrite(wp, "];\n}\n\n");

	return 0;
}

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
	{ "nvram_action", nvram_action_hook},
	{ "nf_values", nf_values_hook},
	{ "get_parameter", ej_get_parameter},
	{ "get_nvram_list", ej_get_nvram_list},
	{ "get_flash_time", ej_get_flash_time},
	{ "get_static_client", ej_get_static_client},
	{ "get_static_ccount", ej_get_static_ccount},
	{ "get_vpns_client", ej_get_vpns_client},
	{ "select_channel", ej_select_channel},
	{ "wl_auth_list", ej_wl_auth_list},
	{ "wl_scan_5g", ej_wl_scan_5g},
	{ "wl_scan_2g", ej_wl_scan_2g},
	{ "wl_bssid_5g", ej_wl_bssid_5g},
	{ "wl_bssid_2g", ej_wl_bssid_2g},
	{ "shown_language_option", ej_shown_language_option},
	{ "hardware_pins", ej_hardware_pins_hook},
	{ "detect_internet", ej_detect_internet_hook},
	{ "dump_syslog", ej_dump_syslog_hook},
	{ "get_usb_ports_info", ej_get_usb_ports_info},
	{ "disk_pool_mapping_info", ej_disk_pool_mapping_info},
	{ "available_disk_names_and_sizes", ej_available_disk_names_and_sizes},
#if (BOARD_NUM_USB_PORTS > 0)
	{ "get_usb_share_list", ej_get_usb_share_list},
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
	{ "openvpn_srv_cert_hook", openvpn_srv_cert_hook},
	{ "openvpn_cli_cert_hook", openvpn_cli_cert_hook},
	{ NULL, NULL }
};

