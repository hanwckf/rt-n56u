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
#include <net/ethernet.h>
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

#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>

#include "initial_web_hook.h"
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

static unsigned long restart_needed_bits = 0;
static unsigned int restart_total_time = 0; 

int action;
char *serviceId;
char *next_host;
int delMap[MAX_GROUP_COUNT+2];
char SystemCmd[128];

extern int change_passwd;
#if defined (SUPPORT_HTTPS)
extern int http_is_ssl;
#endif

static void
nvram_commit_safe()
{
	if (nvram_get_int("nvram_manual") == 1)
		return;
	
	nvram_commit();
}

void
sys_reboot()
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
		eval("start_ddns");
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

void websScan(char_t *str)
{
#define SCAN_MAX_VALUE_LEN 256
	unsigned int i, flag, i_len;
	char_t *v1, *v2, *v3, *sp;
	char_t groupid[64];
	char_t value[SCAN_MAX_VALUE_LEN];
	
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

void websApply(webs_t wp, char_t *url)
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
ej_nvram_get_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_get_ddns(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_match_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_double_match_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_match_both_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_get_list_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_get_buf_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_get_table_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_match_list_x(int eid, webs_t wp, int argc, char_t **argv)
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
ej_select_channel(int eid, webs_t wp, int argc, char_t **argv)
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
ej_nvram_char_to_ascii(int eid, webs_t wp, int argc, char_t **argv)
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
ej_uptime(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[512];
	int ret;
	char *str = file2str("/proc/uptime");
	time_t tm;

	time(&tm);
	sprintf(buf, rfctime(&tm));

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

static int
ej_sysuptime(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret=0;
	char *str = file2str("/proc/uptime");

	if (str) {
		unsigned long up = atoi(str);
		free(str);

		char lease_buf[128];
		memset(lease_buf, 0, sizeof(lease_buf));
		reltime(up, lease_buf);
		ret = websWrite(wp, "%s since boot", lease_buf);
	}

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
ej_dump(int eid, webs_t wp, int argc, char_t **argv)
{	
	char filename[128];
	char *file,*script;
	int ret;

	if (ejArgs(argc, argv, "%s %s", &file, &script) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	// run scrip first to update some status
	if (strcmp(script,"")!=0) sys_script(script); 
 
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

static int
ej_load(int eid, webs_t wp, int argc, char_t **argv)
{
	char *script;

	if (ejArgs(argc, argv, "%s", &script) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	sys_script(script);

	return (websWrite(wp, "%s", ""));
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

enum {
	NOTHING,
	REBOOT,
	RESTART,
};

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

static char *error_msg_console = NULL, *error_msg = NULL;

static char *get_msg_from_dict(char *lang, const char *const msg_name) {
#define MAX_FILE_LENGTH 512
	char current_dir[MAX_FILE_LENGTH];
	char dict_file[MAX_FILE_LENGTH], *dict_info;
	char *follow_info, *follow_info_end, *target;
	int len;
	
	if (lang == NULL || strlen(lang) == 0)
		lang = "EN";
	
	memset(dict_file, 0, MAX_FILE_LENGTH);
	memset(current_dir, 0, MAX_FILE_LENGTH);
	getcwd(current_dir, MAX_FILE_LENGTH);
	sprintf(dict_file, "%s/%s.dict", current_dir, lang);
	
	dict_info = read_whole_file(dict_file);
	if (dict_info == NULL) {
		csprintf("No dictionary file, \"%s\".\n", dict_file);
		return NULL;
	}
	
	follow_info = strstr(dict_info, msg_name);
	if (follow_info == NULL) {
		csprintf("No \"%s\" in the dictionary file.\n", msg_name);
		free(dict_info);
		return NULL;
	}
	
	follow_info += strlen(msg_name)+strlen("=");
	follow_info_end = follow_info;
	while (*follow_info_end != 0 && *follow_info_end != '\n')
		++follow_info_end;
	
	len = follow_info_end-follow_info;
	
	target = (char *)malloc(sizeof(char)*(len+1));
	if (target == NULL) {
		csprintf("No memory \"target\".\n");
		free(dict_info);
		return NULL;
	}
	if (len > 0)
		strncpy(target, follow_info, len);
	target[len] = 0;
	
	free(dict_info);
	return target;
}

static void show_error_msg(const char *const msg_num) {
	char msg_name[32];
	
	memset(msg_name, 0, 32);
	sprintf(msg_name, "ALERT_OF_ERROR_%s", msg_num);
	
	error_msg_console = get_msg_from_dict("", msg_name);
	error_msg = get_msg_from_dict(nvram_safe_get("preferred_lang"), msg_name);
	
	return;
}

static void clean_error_msg() {
	if (error_msg_console != NULL)
		free(error_msg_console);
	
	if (error_msg != NULL)
		free(error_msg);
	
	return;
}

#define WIFI_COMMON_CHANGE_BIT	(1<<0)
#define WIFI_RADIO_CONTROL_BIT	(1<<1)
#define WIFI_GUEST_CONTROL_BIT	(1<<2)
#define WIFI_SCHED_CONTROL_BIT	(1<<3)

static int nvram_modified = 0;
static int wl_modified = 0;
static int rt_modified = 0;

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
					}
					else if (!strcmp(v->name, "wl_guest_ssid"))
					{
						set_wifi_ssid(IFNAME_5G_GUEST, value);
					}
					else
#endif
					if (!strcmp(v->name, "wl_TxPower"))
					{
						set_wifi_param_int(IFNAME_5G_MAIN, "TxPower", value, 0, 100);
					}
					else if (!strcmp(v->name, "wl_greenap"))
					{
						set_wifi_param_int(IFNAME_5G_MAIN, "GreenAP", value, 0, 1);
					}
					else if (!strcmp(v->name, "wl_IgmpSnEnable"))
					{
						set_wifi_param_int(IFNAME_5G_MAIN, "IgmpSnEnable", value, 0, 1);
					}
					else if (!strcmp(v->name, "wl_mcastrate"))
					{
						set_wifi_mrate(IFNAME_5G_MAIN, value);
					}
					else if (!strcmp(v->name, "wl_guest_mcs_mode"))
					{
						set_wifi_mcs_mode(IFNAME_5G_GUEST, value);
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
					}
					else if (!strcmp(v->name, "rt_guest_ssid"))
					{
						set_wifi_ssid(IFNAME_2G_GUEST, value);
					}
					else
#endif
					if (!strcmp(v->name, "rt_TxPower"))
					{
						set_wifi_param_int(IFNAME_2G_MAIN, "TxPower", value, 0, 100);
					}
					else if (!strcmp(v->name, "rt_greenap"))
					{
						set_wifi_param_int(IFNAME_2G_MAIN, "GreenAP", value, 0, 1);
					}
					else if (!strcmp(v->name, "rt_IgmpSnEnable"))
					{
						set_wifi_param_int(IFNAME_2G_MAIN, "IgmpSnEnable", value, 0, 1);
					}
					else if (!strcmp(v->name, "rt_mcastrate"))
					{
						set_wifi_mrate(IFNAME_2G_MAIN, value);
					}
					else if (!strcmp(v->name, "rt_guest_mcs_mode"))
					{
						set_wifi_mcs_mode(IFNAME_2G_GUEST, value);
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

static int update_variables_ex(int eid, webs_t wp, int argc, char_t **argv) {
	int sid;
	char *action_mode;
	char *sid_list;
	char *script;
	int result;
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
		unsigned int max_time = 1;
		if ((restart_needed_bits & RESTART_REBOOT) != 0)
			restart_total_time = MAX(ITVL_RESTART_REBOOT, restart_total_time);
		if ((restart_needed_bits & RESTART_IPV6) != 0)
			restart_total_time = MAX(ITVL_RESTART_IPV6, restart_total_time);
		if ((restart_needed_bits & RESTART_LAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_LAN, restart_total_time);
		if ((restart_needed_bits & RESTART_DHCPD) != 0)
			restart_total_time = MAX(ITVL_RESTART_DHCPD, restart_total_time);
		if ((restart_needed_bits & RESTART_MODEM) != 0)
			restart_total_time = MAX(ITVL_RESTART_MODEM, restart_total_time);
		if ((restart_needed_bits & RESTART_WAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_WAN, restart_total_time);
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
		if ((restart_needed_bits & RESTART_DNS) != 0)
			restart_total_time = MAX(ITVL_RESTART_DNS, restart_total_time);
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
		if ((restart_needed_bits & RESTART_WDG_CPU) != 0)
			restart_total_time = MAX(ITVL_RESTART_WDG_CPU, restart_total_time);
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

static int asus_nvram_commit(int eid, webs_t wp, int argc, char_t **argv) {
	if (nvram_modified) {
		nvram_modified = 0;
		nvram_commit_safe();
	}
	
	return 0;
}

extern long uptime(void);

static int ej_notify_services(int eid, webs_t wp, int argc, char_t **argv) {
	restart_total_time = 0;
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
				restart_needed_bits &= ~(u32)RESTART_DNS;		// dnsmasq already re-started (RESTART_IPV6)
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
				restart_needed_bits &= ~(u32)RESTART_DNS;		// dnsmasq already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_UPNP;		// miniupnpd already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_VPNSVR;		// vpn server already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_IPTV;		// igmpproxy already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_LAN)
			}
			if ((restart_needed_bits & RESTART_MODEM) != 0) {
				notify_rc("restart_modem");
				restart_needed_bits &= ~(u32)RESTART_MODEM;
				restart_needed_bits &= ~(u32)RESTART_WAN;		// wan already re-started (RESTART_MODEM)
			}
			if ((restart_needed_bits & RESTART_WAN) != 0) {
				notify_rc("restart_whole_wan");
				restart_needed_bits &= ~(u32)RESTART_WAN;
				restart_needed_bits &= ~(u32)RESTART_IPTV;		// iptv already re-started (RESTART_WAN)
				restart_needed_bits &= ~(u32)RESTART_SWITCH_VLAN;	// vlan filter already re-started (RESTART_WAN)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_WAN)
			}
			if ((restart_needed_bits & RESTART_IPTV) != 0) {
				notify_rc("restart_iptv");
				restart_needed_bits &= ~(u32)RESTART_IPTV;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_IPTV)
			}
			if ((restart_needed_bits & RESTART_DHCPD) != 0) {
				notify_rc("restart_dhcpd");
				restart_needed_bits &= ~(u32)RESTART_DHCPD;
			}
			if ((restart_needed_bits & RESTART_FTPSAMBA) != 0) {
				notify_rc("restart_cifs");
				restart_needed_bits &= ~(u32)RESTART_FTPSAMBA;
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
			if ((restart_needed_bits & RESTART_DDNS) != 0) {
				notify_rc("restart_ddns");
				restart_needed_bits &= ~(u32)RESTART_DDNS;
			}
			if ((restart_needed_bits & RESTART_HTTPD) != 0) {
				notify_rc("restart_httpd");
				restart_needed_bits &= ~(u32)RESTART_HTTPD;
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_HTTPD)
			}
			if ((restart_needed_bits & RESTART_DNS) != 0) {
				notify_rc("restart_dns");
				restart_needed_bits &= ~(u32)RESTART_DNS;
			}
			if ((restart_needed_bits & RESTART_UPNP) != 0) {
				notify_rc("restart_upnp");
				restart_needed_bits &= ~(u32)RESTART_UPNP;
			}
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
			if ((restart_needed_bits & RESTART_SWITCH) != 0) {
				notify_rc("restart_switch_config");
				restart_needed_bits &= ~(u32)RESTART_SWITCH;
			}
			if ((restart_needed_bits & RESTART_SWITCH_VLAN) != 0) {
				notify_rc("restart_switch_vlan");
				restart_needed_bits &= ~(u32)RESTART_SWITCH_VLAN;
			}
			if ((restart_needed_bits & RESTART_SYSLOG) != 0) {
				notify_rc("restart_syslog");
				restart_needed_bits &= ~(u32)RESTART_SYSLOG;
			}
			if ((restart_needed_bits & RESTART_WDG_CPU) != 0) {
				notify_rc("restart_wdg_cpu");
				restart_needed_bits &= ~(u32)RESTART_WDG_CPU;
			}
			if ((restart_needed_bits & RESTART_FIREWALL) != 0) {
				notify_rc("restart_firewall");
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;
			}
			if ((restart_needed_bits & RESTART_NTPC) != 0) {
				notify_rc("restart_ntpc");
				restart_needed_bits &= ~(u32)RESTART_NTPC;
			}
			if ((restart_needed_bits & RESTART_NFS) != 0) {
				notify_rc("restart_nfs");
				restart_needed_bits &= ~(u32)RESTART_NFS;
			}
			if ((restart_needed_bits & RESTART_TIME) != 0) {
				notify_rc("restart_time");
				restart_needed_bits &= ~(u32)RESTART_TIME;
			}
			if ((restart_needed_bits & RESTART_SPOOLER) != 0) {
				notify_rc("restart_spooler");
				restart_needed_bits &= ~(u32)RESTART_SPOOLER;
			}
			if ((restart_needed_bits & RESTART_HDDTUNE) != 0) {
				notify_rc("restart_hddtune");
				restart_needed_bits &= ~(u32)RESTART_HDDTUNE;
			}
			if ((restart_needed_bits & RESTART_SYSCTL) != 0) {
				notify_rc("restart_sysctl");
				restart_needed_bits &= ~(u32)RESTART_SYSCTL;
			}
			if ((restart_needed_bits & RESTART_WIFI) != 0) {
				if (wl_modified) {
					if (wl_modified & WIFI_COMMON_CHANGE_BIT)
						notify_rc("restart_wifi_wl");
					else {
						if (rt_modified & WIFI_RADIO_CONTROL_BIT)
							notify_rc("control_wifi_radio_wl");
						
						if (wl_modified & WIFI_GUEST_CONTROL_BIT)
							notify_rc("control_wifi_guest_wl");
						
						if (wl_modified & WIFI_SCHED_CONTROL_BIT)
							nvram_set_int_temp("reload_svc_wl", 1);
					}
					wl_modified = 0;
				}
				
				if (rt_modified) {
					if (rt_modified & WIFI_COMMON_CHANGE_BIT)
						notify_rc("restart_wifi_rt");
					else {
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

static int detect_if_wan(int eid, webs_t wp, int argc, char_t **argv) {
	int phy_wan = 0;

	if (!get_ap_mode()) {
		phy_wan = (get_wan_phy_connected()) ? 1 : 0;
		kill_pidfile_s("/var/run/detect_internet.pid", SIGHUP);
	}

	websWrite(wp, "%d", phy_wan);

	return 0;
}

static int detect_dhcp_pppoe(int eid, webs_t wp, int argc, char_t **argv) {
	if (!get_ap_mode()) {
		if (get_usb_modem_wan(0))
			websWrite(wp, "modem");
		else
			websWrite(wp, "dhcp");
	} else {
		websWrite(wp, "AP mode");
	}

	return 0;
}

int file_to_buf(char *path, char *buf, int len) {
	FILE *fp;
	memset(buf, 0 , len);
	
	if ((fp = fopen(path, "r")) != NULL) {
		fgets(buf, len, fp);
		fclose(fp);
		
		return 1;
	}
	
	return 0;
}

int get_ppp_pid(char *conntype) {
	int pid = -1;
	char tmp[80], tmp1[80];
	
	snprintf(tmp, sizeof(tmp), "/var/run/%s.pid", conntype);
	file_to_buf(tmp, tmp1, sizeof(tmp1));
	pid = atoi(tmp1);
	
	return pid;
}

/* Find process name by pid from /proc directory */
char *find_name_by_proc(int pid) {
	FILE *fp;
	char line[254];
	char filename[80];
	static char name[80];
	
	snprintf(filename, sizeof(filename), "/proc/%d/status", pid);
	
	if ((fp = fopen(filename, "r")) != NULL) {
		fgets(line, sizeof(line), fp);
		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(line, "%*s %s", name);
		fclose(fp);
		return name;
	}
	
	return "";
}

int check_ppp_exist() {
	DIR *dir;
	struct dirent *dent;
	char task_file[64], cmdline[64];
	int pid, fd;
	
	if ((dir = opendir("/proc")) == NULL) {
		perror("open proc");
		return -1;
	}
	
	while((dent = readdir(dir)) != NULL) {
		if ((pid = atoi(dent->d_name)) > 1) {
			memset(task_file, 0, 64);
			sprintf(task_file, "/proc/%d/cmdline", pid);
			if ((fd = open(task_file, O_RDONLY)) > 0) {
				memset(cmdline, 0, 64);
				read(fd, cmdline, 64);
				close(fd);
				
				if (strstr(cmdline, "pppoecd")
						|| strstr(cmdline, "pppd")
						) {
					closedir(dir);
					return 0;
				}
			}
			else
				printf("cannot open %s\n", task_file);
		}
	}
	closedir(dir);
	
	return -1;
}

int check_subnet() {
	if (!strcmp(nvram_safe_get("wan_subnet_t"), nvram_safe_get("lan_subnet_t")))
		return 1;
	else
		return 0;
}

int
get_if_status(const char *wan_ifname)
{
	int s, status;
	struct ifreq ifr;
	struct sockaddr_in *wan_addr_in;
	
	if (get_ap_mode())
		return 0;
	
	status = 0;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s >= 0) {
		/* Check for valid IP address */
		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
		
		if (ioctl(s, SIOCGIFADDR, &ifr) == 0) {
			wan_addr_in = (struct sockaddr_in *)&ifr.ifr_addr;
			if (wan_addr_in->sin_addr.s_addr != INADDR_ANY &&
			    wan_addr_in->sin_addr.s_addr != INADDR_NONE)
				status = 1;
		}
		
		close(s);
	}
	
	return status;
}

static int wanlink_hook(int eid, webs_t wp, int argc, char_t **argv) {
	FILE *fp;
	char type[32], dns[256], dns_item[80], wan_mac[18], statusstr[32], etherlink[32] = {0};
	int status = 0, unit, is_first, i_wan_src_phy, is_wan_modem;
	long ppp_time;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan0_ip, *wanx_ip, *wan0_gw, *wanx_gw, *wan_ip6, *lan_ip6;
#if defined (USE_IPV6)
	int ipv6_type;
	char *wan6_ifname = NULL;
	char addr6_wan[INET6_ADDRSTRLEN], addr6_lan[INET6_ADDRSTRLEN];
#endif
	snprintf(wan_mac, sizeof(wan_mac), "%s", nvram_safe_get("wan_hwaddr"));
	
	wanx_ip = "";
	wanx_gw = "";
	wan_ip6 = "";
	lan_ip6 = "";
	ppp_time = 0;
	
	/* current unit */
	if ((unit = nvram_get_int("wan_unit")) < 0)
		unit = 0;
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);
	
	statusstr[0] = 0;
	
	is_wan_modem = get_usb_modem_wan(0);
	
	if (is_wan_modem)
	{
		if (nvram_match("modem_type", "3")) {
			struct ifreq ifr;
			char *ndis_ifname = nvram_safe_get("wan_ifname_t");
			if (isUsbNetIf(ndis_ifname)) {
				status = get_if_status(ndis_ifname);
				if (get_if_hwaddr(ndis_ifname, &ifr) == 0) {
					sprintf(wan_mac, "%02X:%02X:%02X:%02X:%02X:%02X", 
						(unsigned char)ifr.ifr_hwaddr.sa_data[0],
						(unsigned char)ifr.ifr_hwaddr.sa_data[1],
						(unsigned char)ifr.ifr_hwaddr.sa_data[2],
						(unsigned char)ifr.ifr_hwaddr.sa_data[3],
						(unsigned char)ifr.ifr_hwaddr.sa_data[4],
						(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
				}
			}
		}
		else {
#if defined (USE_IPV6)
			if (nvram_get_int("ip6_wan_if") == 0)
				wan6_ifname = IFNAME_RAS;
#endif
			status = get_if_status(IFNAME_RAS);
			ppp_time = nvram_get_int(strcat_r(prefix, "time", tmp));
			
			// Dual access with 3G Modem
			if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp") ||
			    nvram_match(strcat_r(prefix, "proto", tmp), "l2tp") ||
			    nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") )
			{
				if (nvram_match("link_wan", "1")) {
					wanx_ip = nvram_safe_get("wanx_ipaddr");
					wanx_gw = nvram_safe_get("wanx_gateway");
				}
			}
		}
	}
	else
	if (!get_wan_phy_connected()) {
		status = 0;
		strcpy(statusstr, "Cable is not attached");
	}
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp") ||
	         nvram_match(strcat_r(prefix, "proto", tmp), "l2tp") ||
	         nvram_match(strcat_r(prefix, "proto", tmp), "pppoe") )
	{
#if defined (USE_IPV6)
		if (nvram_get_int("ip6_wan_if") == 0)
			wan6_ifname = IFNAME_PPP;
#endif
		status = get_if_status(IFNAME_PPP);
		ppp_time = nvram_get_int(strcat_r(prefix, "time", tmp));
		
		wanx_ip = nvram_safe_get("wanx_ipaddr");
		wanx_gw = nvram_safe_get("wanx_gateway");
	}
	else {
		if (check_subnet())
			status = 0;
		else
			status = get_if_status(nvram_safe_get("wan0_ifname"));
	}
	
	if (ppp_time > 0)
		ppp_time = uptime() - ppp_time;
	
	if ( !statusstr[0] ) {
		if (status)
			strcpy(statusstr, "Connected");
		else
			strcpy(statusstr, "Disconnected");
	}
	
	if (is_wan_modem)
	{
		if(nvram_match("modem_type", "3"))
			strcpy(type, "Modem (NDIS/RNDIS)");
		else
			strcpy(type, "Modem (RAS)");
	}
	else
	if (nvram_match(strcat_r(prefix, "proto", tmp), "pppoe"))
	{
		strcpy(type, "PPPoE");
	}
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "pptp"))
	{
		strcpy(type, "PPTP");
	}
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "l2tp"))
	{
		strcpy(type, "L2TP");
	}
	else if (nvram_match(strcat_r(prefix, "proto", tmp), "static"))
	{
		strcpy(type, "Static IP");
	}
	else // dhcp
	{
		strcpy(type, "Automatic IP");
	}
	
	if (status == 0)
	{
		wan0_ip = "---";
		wan0_gw = "---";
	}
	else
	{
		wan0_ip = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));
		wan0_gw = nvram_safe_get(strcat_r(prefix, "gateway", tmp));
		if (strcmp(wan0_ip, "0.0.0.0") == 0 || !(*wan0_ip))
			wan0_ip = "---";
		if (strcmp(wan0_gw, "0.0.0.0") == 0 || !(*wan0_gw))
			wan0_gw = "---";
	}
	
#if defined (USE_IPV6)
	ipv6_type = get_ipv6_type();
	if (ipv6_type != IPV6_DISABLED)
	{
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
			wan6_ifname = IFNAME_SIT;
		else {
			if (!wan6_ifname)
				wan6_ifname = nvram_safe_get("wan0_ifname");
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
	
	if (strcmp(wanx_ip, "0.0.0.0") == 0)
		wanx_ip = "";
	if (strcmp(wanx_gw, "0.0.0.0") == 0 || !(*wanx_gw))
		wanx_gw = "---";
	
	dns[0] = 0;
	fp = fopen("/etc/resolv.conf", "r");
	if (fp) {
		is_first = 1;
		dns_item[0] = 0;
		while(fscanf(fp, "nameserver %s\n", dns_item) > 0) {
			if (*dns_item && strcmp(dns_item, "127.0.0.1") != 0) {
				if (is_first)
					is_first = 0;
				else
					strcat(dns, "<br/>");
				strcat(dns, dns_item);
			}
		}
		fclose(fp);
	}
	
	if (!(*dns))
		strcpy(dns, "---");
	
	i_wan_src_phy = nvram_get_int("wan_src_phy");
	fill_eth_port_status(i_wan_src_phy, etherlink);
	
	websWrite(wp, "function wanlink_status() { return %d;}\n", status);
	websWrite(wp, "function wanlink_statusstr() { return '%s';}\n", statusstr);
	websWrite(wp, "function wanlink_etherlink() { return '%s';}\n", etherlink);
	websWrite(wp, "function wanlink_time() { return %ld;}\n", (ppp_time > 0) ? ppp_time : 0);
	websWrite(wp, "function wanlink_type() { return '%s';}\n", type);
	websWrite(wp, "function wanlink_ip4_wan() { return '%s';}\n", wan0_ip);
	websWrite(wp, "function wanlink_gw4_wan() { return '%s';}\n", wan0_gw);
	websWrite(wp, "function wanlink_ip4_man() { return '%s';}\n", wanx_ip);
	websWrite(wp, "function wanlink_gw4_man() { return '%s';}\n", wanx_gw);
	websWrite(wp, "function wanlink_ip6_wan() { return '%s';}\n", wan_ip6);
	websWrite(wp, "function wanlink_ip6_lan() { return '%s';}\n", lan_ip6);
	websWrite(wp, "function wanlink_dns() { return '%s';}\n", dns);
	websWrite(wp, "function wanlink_mac() { return '%s';}\n", wan_mac);

	return 0;
}

static int lanlink_hook(int eid, webs_t wp, int argc, char_t **argv) 
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

static int wan_action_hook(int eid, webs_t wp, int argc, char_t **argv) 
{
	char *action;
	int needed_seconds = 0;
	
	// assign control variables
	action = websGetVar(wp, "wanaction", "");
	if (strlen(action) <= 0) {
		fprintf(stderr, "No connect action in wan_action_hook!\n");
		return -1;
	}
	
	fprintf(stderr, "wan action: %s\n", action);
	
	if (!strcmp(action, "Connect")) {
		notify_rc("manual_wan_connect");
		needed_seconds = 5;
	}
	else if (!strcmp(action, "Disconnect")) {
		notify_rc("manual_wan_disconnect");
		needed_seconds = 3;
	}
	
	websWrite(wp, "<script>restart_needed_time(%d);</script>\n", needed_seconds);
	
	return 0;
}

static int wol_action_hook(int eid, webs_t wp, int argc, char_t **argv) 
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

static int nvram_action_hook(int eid, webs_t wp, int argc, char_t **argv) 
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


static int nf_values_hook(int eid, webs_t wp, int argc, char_t **argv) 
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

static int firmware_caps_hook(int eid, webs_t wp, int argc, char_t **argv) 
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
#if defined(USE_USB3)
	int has_usb3 = 1;
#else
	int has_usb3 = 0;
#endif
#if defined(SUPPORT_HTTPS)
	int has_https = 1;
#else
	int has_https = 0;
#endif
#if defined(BOARD_GPIO_LED_ALL)
	int has_led_all = 1;
#else
	int has_led_all = 0;
#endif
#if defined(BOARD_GPIO_LED_WIFI)
	int has_led_wifi = 1;
#else
	int has_led_wifi = 0;
#endif
#if defined(BOARD_GPIO_BTN_WPS)
	int has_but_wps = 1;
#else
	int has_but_wps = 0;
#endif
#if defined(USE_RT3352_MII)
	int has_inic_mii = 1;
#else
	int has_inic_mii = 0;
#endif
#if defined(USE_RTL8367)
	int use_switch_type = 0; // Realtek RTL8367
#elif defined(USE_MTK_ESW)
	int use_switch_type = 1; // Mediatek MT7620 Embedded GSW
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
		"function support_https() { return %d;}\n"
		"function support_min_vlan() { return %d;}\n"
		"function support_pcie_usb3() { return %d;}\n"
		"function support_but_wps() { return %d;}\n"
		"function support_led_phy() { return %d;}\n"
		"function support_led_all() { return %d;}\n"
		"function support_switch_type() { return %d;}\n"
		"function support_2g_apcli_only() { return %d;}\n"
		"function support_5g_radio() { return %d;}\n"
		"function support_5g_stream_tx() { return %d;}\n"
		"function support_5g_stream_rx() { return %d;}\n"
		"function support_2g_stream_tx() { return %d;}\n"
		"function support_2g_stream_rx() { return %d;}\n",
		has_ipv6,
		has_ipv6_ppe,
		has_https,
		min_vlan_ext,
		has_usb3,
		has_but_wps,
		BOARD_NUM_ETH_LEDS,
		has_led_all|has_led_wifi,
		use_switch_type,
		(has_inic_mii) ? 0 : 1,
		BOARD_HAS_5G_RADIO,
		BOARD_NUM_ANT_5G_TX,
		BOARD_NUM_ANT_5G_RX,
		BOARD_NUM_ANT_2G_TX,
		BOARD_NUM_ANT_2G_RX
	);

	return 0;
}

static int openvpn_srv_cert_hook(int eid, webs_t wp, int argc, char_t **argv)
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

static int openvpn_cli_cert_hook(int eid, webs_t wp, int argc, char_t **argv)
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

static int vpnc_state_hook(int eid, webs_t wp, int argc, char_t **argv)
{
	int i_vpnc_status = (nvram_get_int("vpnc_state_t") == 1) ? 1 : 0;

	websWrite(wp, "function vpnc_state() { return %d;}\n", i_vpnc_status);

	return 0;
}

static int ej_get_parameter(int eid, webs_t wp, int argc, char_t **argv) {
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

static int login_state_hook(int eid, webs_t wp, int argc, char_t **argv) {
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

static int ej_get_nvram_list(int eid, webs_t wp, int argc, char_t **argv) {
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

static int ej_dhcp_leases(int eid, webs_t wp, int argc, char_t **argv) {
	FILE *fp = NULL;
	int i, firstRow;
	char buff[256], dh_lease[32], dh_mac[32], dh_ip[32], dh_host[64], dh_sid[32];
	
	/* Write out leases file */
	if (!(fp = fopen("/tmp/dnsmasq.leases", "r")))
		return 0;
	
	firstRow = 1;
	buff[0] = 0;
	while (fgets(buff, sizeof(buff), fp))
	{
		dh_lease[0] = 0;
		dh_mac[0] = 0;
		dh_ip[0] = 0;
		dh_host[0] = 0;
		dh_sid[0] = 0;
		if (sscanf(buff, "%s %s %s %s %s", dh_lease, dh_mac, dh_ip, dh_host, dh_sid) != 5)
			continue;
		
		if (!dh_mac[0] || !strcmp(dh_mac, "00:00:00:00:00:00"))
			continue;
		
		if (firstRow == 1)
			firstRow = 0;
		else
			websWrite(wp, ", ");
		
		if (!strcmp(dh_host, "*"))
			dh_host[0] = 0;
		
		// cut hostname to 18 chars
		dh_host[18] = 0;
		
		// convert MAC to upper case
		for (i=0; i<strlen(dh_mac); i++)
			dh_mac[i] = toupper(dh_mac[i]);
		
		websWrite(wp, "[\"%s\", \"%s\", \"%s\"]", dh_host, dh_mac, dh_ip);
	}
	
	fclose(fp);
	
	return 0;
}

static int ej_get_arp_table(int eid, webs_t wp, int argc, char_t **argv) {
	const int MAX = 80;
	const int FIELD_NUM = 6;
	const int VALUELEN = 18;
	char buffer[MAX], values[FIELD_NUM][VALUELEN];
	int num, firstRow;
	
	FILE *fp = fopen("/proc/net/arp", "r");
	if (fp) {
		memset(buffer, 0, MAX);
		memset(values, 0, FIELD_NUM*VALUELEN);
		
		firstRow = 1;
		while (fgets(buffer, MAX, fp)) {
			if (strstr(buffer, IFNAME_BR) && !strstr(buffer, "00:00:00:00:00:00")) {
				if (firstRow == 1)
					firstRow = 0;
				else
					websWrite(wp, ", ");
				
				if ((num = sscanf(buffer, "%s%s%s%s%s%s", values[0], values[1], values[2], values[3], values[4], values[5])) == FIELD_NUM) {
					websWrite(wp, "[\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\"]", values[0], values[1], values[2], values[3], values[4], values[5]);
				}
				
				memset(values, 0, FIELD_NUM*VALUELEN);
			}
			
			memset(buffer, 0, MAX);
		}
		
		fclose(fp);
	}
	
	return 0;
}

// for detect static IP's client.
static int ej_get_static_client(int eid, webs_t wp, int argc, char_t **argv) 
{
	FILE *fp;
	char buf[1024], *head, *tail, field[1024];
	int i, lock, len, first_client, first_field;

	lock = file_lock("networkmap");

	fp = fopen("/tmp/static_ip.inf", "r");
	if (fp) {
		first_client = 1;
		while (fgets(buf, sizeof(buf), fp)) {
			if (first_client == 1)
				first_client = 0;
			else
				websWrite(wp, ", ");
			
			len = strlen(buf);
			buf[len-1] = ',';
			head = buf;
			first_field = 1;
			for (i = 0; i < 7; ++i) {
				tail = strchr(head, ',');
				if (tail != NULL) {
					memset(field, 0, sizeof(field));
					strncpy(field, head, (tail-head));
				}
				
				if (first_field == 1) {
					first_field = 0;
					websWrite(wp, "[");
				}
				else
					websWrite(wp, ", ");
				
				if (strlen(field) > 0)
					websWrite(wp, "\"%s\"", field);
				else
					websWrite(wp, "null");
				
				//if (tail+1 != NULL)
					head = tail+1;
				
				if (i == 6)
					websWrite(wp, "]");
			}
		}
		
		fclose(fp);
	}

	file_unlock(lock);

	return 0;
}

static int ej_get_static_ccount(int eid, webs_t wp, int argc, char_t **argv) 
{
	FILE *fp;
	char buf[1024];
	int lock, ccount;

	ccount = 0;

	lock = file_lock("networkmap");

	fp = fopen("/tmp/static_ip.inf", "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp))
			ccount++;
		
		fclose(fp);
	}

	file_unlock(lock);

	websWrite(wp, "%d", ccount);

	return 0;
}


static int ej_get_vpns_client(int eid, webs_t wp, int argc, char_t **argv) 
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

int is_interface_up(const char *ifname)
{
	int sockfd;
	struct ifreq ifreq;
	int iflags = 0;
	
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		return 0;
	}
	
	strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFFLAGS, &ifreq) < 0) {
		iflags = 0;
	} else {
		iflags = ifreq.ifr_flags;
	}
	
	close(sockfd);
	
	if (iflags & IFF_UP)
		return 1;
	
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

static int ej_system_status_hook(int eid, webs_t wp, int argc, char_t **argv)
{
	struct sysinfo info;
	struct cpu_stats cpu;
	struct mem_stats mem;
	struct wifi_stats wifi2;
	struct wifi_stats wifi5;
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

	websWrite(wp, "{\"lavg\": \"%u.%02u %u.%02u %u.%02u\", "
			"\"uptime\": {\"days\": %lu, \"hours\": %lu, \"minutes\": %lu}, "
			"\"ram\": {\"total\": %lu, \"used\": %lu, \"free\": %lu, \"buffers\": %lu, \"cached\": %lu}, "
			"\"swap\": {\"total\": %lu, \"used\": %lu, \"free\": %lu}, "
			"\"cpu\": {\"busy\": %u, \"user\": %u, \"nice\": %u, \"system\": %u, "
				  "\"idle\": %u, \"iowait\": %u, \"irq\": %u, \"sirq\": %u}, "
			"\"wifi2\": {\"state\": %d, \"guest\": %d}, "
			"\"wifi5\": {\"state\": %d, \"guest\": %d}}",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]),
			updays, uphours, upminutes,
			mem.total, (mem.total - mem.free), mem.free, mem.buffers, mem.cached,
			mem.sw_total, (mem.sw_total - mem.sw_free), mem.sw_free,
			u_busy, u_user, u_nice, u_system, u_idle, u_iowait, u_irq, u_sirq,
			wifi2.radio, wifi2.ap_guest, wifi5.radio, wifi5.ap_guest
		);

	return 0;
}

static int ej_disk_pool_mapping_info(int eid, webs_t wp, int argc, char_t **argv) {
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int first;

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		websWrite(wp, "%s", initial_disk_pool_mapping_info());
		return -1;
	}

	char *Ptr;

	websWrite(wp, "function pool_names() {\n");
	websWrite(wp, "    return [");

	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			
			if (follow_partition->mount_point == NULL) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			
			Ptr = rindex(follow_partition->mount_point, '/');
			if (Ptr == NULL) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			
			if (strncmp(follow_partition->mount_point, "/media/", 7) != 0) {
				websWrite(wp, "\"unknown\"");
				continue;
			}
			++Ptr;
			websWrite(wp, "\"%s\"", Ptr);
		}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_types() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			if (follow_partition->mount_point == NULL) {
				websWrite(wp, "\"unknown\"");
				continue;
			}

			websWrite(wp, "\"%s\"", follow_partition->file_system);
		}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_status() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			if (follow_partition->mount_point == NULL) {
				websWrite(wp, "\"unmounted\"");
				continue;
			}

			//if (strcmp(follow_partition->file_system, "ntfs") == 0)
			//	websWrite(wp, "\"ro\"");
			//else
			websWrite(wp, "\"rw\"");
		}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_kilobytes_in_use() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			//printf("[%llu] ", follow_partition->used_kilobytes);	// tmp test
			websWrite(wp, "%llu", follow_partition->used_kilobytes);
		}

	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	disk_info_t *follow_disk2;
	u32 disk_num, pool_num;
	websWrite(wp, "function per_pane_pool_usage_kilobytes(pool_num, disk_num) {\n");
	for (follow_disk = disks_info, pool_num = 0; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++pool_num) {
			websWrite(wp, "    if (pool_num == %d) {\n", pool_num);
			if (follow_partition->mount_point != NULL)
				for (follow_disk2 = disks_info, disk_num = 0; follow_disk2 != NULL; follow_disk2 = follow_disk2->next, ++disk_num) {
					websWrite(wp, "	if (disk_num == %d) {\n", disk_num);
					if (follow_disk2->major == follow_disk->major && follow_disk2->minor == follow_disk->minor)
						websWrite(wp, "	    return [%llu];\n", follow_partition->size_in_kilobytes);
					else
						websWrite(wp, "	    return [0];\n");
					websWrite(wp, "	}\n");
				}
			else
				websWrite(wp, "	return [0];\n");
			websWrite(wp, "    }\n");
		}
	}
	websWrite(wp, "}\n\n");
	free_disk_data(disks_info);

	return 0;
}

static int ej_available_disk_names_and_sizes(int eid, webs_t wp, int argc, char_t **argv) {
	disk_info_t *disks_info, *follow_disk;
	int first;

	websWrite(wp, "function blank_disks() { return [];}\n");
	websWrite(wp, "function blank_disk_interface_names() { return [];}\n");
	websWrite(wp, "function blank_disk_device_names() { return [];}\n");
	websWrite(wp, "function blank_disk_model_info() { return [];}\n");
	websWrite(wp, "function blank_disk_total_size() { return [];}\n");
	websWrite(wp, "function blank_disk_total_mounted_number() { return [];}\n\n");

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		websWrite(wp, "%s", initial_available_disk_names_and_sizes());
		return -1;
	}

	/* show name of the foreign disks */
	websWrite(wp, "function foreign_disks() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%s\"", follow_disk->tag);
	}
	websWrite(wp, "];\n}\n\n");

	/* show interface of the foreign disks */
	websWrite(wp, "function foreign_disk_interface_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%u\"", follow_disk->port_root);
	}
	websWrite(wp, "];\n}\n\n");

	/* show device name of the foreign disks */
	websWrite(wp, "function foreign_disk_device_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%s\"", follow_disk->device);
	}
	websWrite(wp, "];\n}\n\n");

	/* show model info of the foreign disks */
	websWrite(wp, "function foreign_disk_model_info() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"");
		if (follow_disk->vendor != NULL)
			websWrite(wp, "%s", follow_disk->vendor);
		if (follow_disk->model != NULL) {
			if (follow_disk->vendor != NULL)
				websWrite(wp, " ");
			websWrite(wp, "%s", follow_disk->model);
		}
		websWrite(wp, "\"");
	}
	websWrite(wp, "];\n}\n\n");

	/* show total_size of the foreign disks */
	websWrite(wp, "function foreign_disk_total_size() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%llu\"", follow_disk->size_in_kilobytes);
	}
	websWrite(wp, "];\n}\n\n");

	/* show total number of the mounted partition in this foreign disk */
	websWrite(wp, "function foreign_disk_total_mounted_number() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");
		websWrite(wp, "\"%u\"", follow_disk->mounted_number);
	}
	websWrite(wp, "];\n}\n\n");

	free_disk_data(disks_info);

	return 0;
}

static int ej_get_usb_ports_info(int eid, webs_t wp, int argc, char_t **argv){

	int i, idx, first, usb_dev_type[2] = {0};
	char *usb_dev_string[2];
	usb_info_t *usb_info, *follow_usb;

	usb_info = get_usb_info();
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		idx = follow_usb->port_root;
		if (idx == 1 || idx == 2) {
			if (follow_usb->dev_type == DEVICE_TYPE_HUB)
				usb_dev_type[idx-1] |= 0x01;
			else if (follow_usb->dev_type == DEVICE_TYPE_DISK)
				usb_dev_type[idx-1] |= 0x02;
			else if (follow_usb->dev_type == DEVICE_TYPE_PRINTER)
				usb_dev_type[idx-1] |= 0x04;
			else if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY)
				usb_dev_type[idx-1] |= 0x08;
			else if (follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH)
				usb_dev_type[idx-1] |= 0x10;
		}
	}

	for (i = 0; i < 2; i++) {
		if (usb_dev_type[i] & 0x01)
			usb_dev_string[i] = "hub";
		else if (usb_dev_type[i] & 0x04)
			usb_dev_string[i] = "printer";
		else if (usb_dev_type[i] & 0x10)
			usb_dev_string[i] = "modem_eth";
		else if (usb_dev_type[i] & 0x08)
			usb_dev_string[i] = "modem_tty";
		else if (usb_dev_type[i] & 0x02)
			usb_dev_string[i] = "storage";
		else
			usb_dev_string[i] = "unknown";
	}

	/* usb ports num */
	websWrite(wp, "function get_usb_ports_num(){\n");
	websWrite(wp, "    return %u;\n", BOARD_NUM_USB_PORTS);
	websWrite(wp, "}\n\n");

	/* usb device types */
	websWrite(wp, "function get_device_type_usb(port_num){\n");
	websWrite(wp, "    if (port_num == 1)\n");
	websWrite(wp, "        return \"%s\";\n", usb_dev_string[0]);
	websWrite(wp, "    else if (port_num == 2)\n");
	websWrite(wp, "        return \"%s\";\n", usb_dev_string[1]);
	websWrite(wp, "    else\n");
	websWrite(wp, "        return \"%s\";\n", "unknown");
	websWrite(wp, "}\n\n");

	/* printers */
	websWrite(wp, "function printer_ports() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->port_root);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_manufacts() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->manuf) ? follow_usb->manuf : "Unknown");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function printer_models() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_PRINTER) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->product) ? follow_usb->product : "Unknown" );
		}
	}
	websWrite(wp, "];\n}\n\n");

	/* modems */
	websWrite(wp, "function modem_ports() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->port_root);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_devnum() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%u\"", follow_usb->id_devnum);
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_types() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY) ? "RAS" : "NDIS/RNDIS");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_manufacts() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->manuf) ? follow_usb->manuf : "Unknown");
		}
	}
	websWrite(wp, "];\n}\n\n");

	websWrite(wp, "function modem_models() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_usb = usb_info; follow_usb != NULL; follow_usb = follow_usb->next) {
		if (follow_usb->dev_type == DEVICE_TYPE_MODEM_TTY ||
		    follow_usb->dev_type == DEVICE_TYPE_MODEM_ETH) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");
			websWrite(wp, "\"%s\"", (follow_usb->product) ? follow_usb->product : "Unknown" );
		}
	}
	websWrite(wp, "];\n}\n\n");

	free_usb_info(usb_info);

	return 0;
}


int ej_shown_language_option(int eid, webs_t wp, int argc, char **argv) {
	struct language_table *pLang = NULL;
	char lang[4];
	int i, len;
	FILE *fp = fopen("EN.dict", "r");
	char buffer[1024], key[16], target[16];
	char *follow_info, *follow_info_end;

	if (fp == NULL) {
		fprintf(stderr, "No English dictionary!\n");
		return 0;
	}

	memset(lang, 0, 4);
	strcpy(lang, nvram_safe_get("preferred_lang"));

	for (i = 0; i < 21; ++i) {
		memset(buffer, 0, sizeof(buffer));
		if ((follow_info = fgets(buffer, sizeof(buffer), fp)) != NULL) {
			if (strncmp(follow_info, "LANG_", 5))    // 5 = strlen("LANG_")
				continue;

			follow_info += 5;
			follow_info_end = strstr(follow_info, "=");
			len = follow_info_end-follow_info;
			memset(key, 0, sizeof(key));
			strncpy(key, follow_info, len);

			for (pLang = language_tables; pLang->Lang != NULL; ++pLang) {
				if (strcmp(key, pLang->Target_Lang))
					continue;
				follow_info = follow_info_end+1;
				follow_info_end = strstr(follow_info, "\n");
				len = follow_info_end-follow_info;
				memset(target, 0, sizeof(target));
				strncpy(target, follow_info, len);

				if (!strcmp(key, lang))
					websWrite(wp, "<option value=\"%s\" selected>%s</option>\\n", key, target);
				else
					websWrite(wp, "<option value=\"%s\">%s</option>\\n", key, target);
				break;
			}
		}
		else
			break;
	}
	fclose(fp);

	return 0;
}

static int
apply_cgi(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
		char_t *url, char_t *path, char_t *query)
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

static unsigned int 
get_mtd_size(const char *mtd)
{
	FILE *fp;
	char dev[PATH_MAX];
	int i;
	unsigned int mtd_size;

	if ((fp = fopen("/proc/mtd", "r"))) {
		while (fgets(dev, sizeof(dev), fp)) {
			if ((sscanf(dev, "mtd%d: %x", &i, &mtd_size) > 1) && 
			    (mtd_size > (65536*10)) && 
			    (strstr(dev, mtd))) {
				fclose(fp);
				return mtd_size;
			}
		}
		fclose(fp);
	}

	return 0;
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
	char pid_ralink[20], pid_asus[8];
	image_header_t *hdr = (image_header_t *)buf;

	/* check header magic */
	if (SWAP_LONG(hdr->ih_magic) != IH_MAGIC) {
		httpd_log("%s: Incorrect image header!", "Firmware update");
		return -1;
	}

	strncpy(pid_ralink, buf+32, 16);
	strncpy(pid_asus, buf+36, 7);
	pid_ralink[16] = 0;
	pid_asus[7] = 0;

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
do_prf_file(char *url, FILE *stream)
{
	char *nvram_file = PROFILE_FIFO_DOWNLOAD;

	unlink(nvram_file);
	eval("nvram", "save", nvram_file);
	do_file(nvram_file, stream);
}


static char cache_static[] =
"Cache-Control: max-age=2592000\r\n" 
"Expires: Tue, 31 Dec 2013 01:00:00 GMT"
;

static char no_cache_IE9[] =
"X-UA-Compatible: IE=9\r\n"
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;

#if 0
static char no_cache_IE7[] =
"X-UA-Compatible: IE=EmulateIE7\r\n"
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;

static char no_cache[] =
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;
#endif

static void 
do_log_cgi(char *url, FILE *stream)
{
	dump_file(stream, "/tmp/syslog.log");
	fputs("\r\n", stream); /* terminator */
	fputs("\r\n", stream); /* terminator */
}

struct mime_handler mime_handlers[] = {
	{ "Nologin.asp", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "jquery.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**bootstrap.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**engage.itoggle.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
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

	{ "httpd_check.htm", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "**.htm*", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, do_auth },
	{ "**.asp*", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, do_auth },

	{ "**.css", "text/css", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.png", "image/png", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.gif", "image/gif", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.jpg", "image/jpeg", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.ico", "image/x-icon", cache_static, NULL, do_file, NULL }, // 2013.04 Eagle23

	{ "**.svg", "image/svg+xml", NULL, NULL, do_file, NULL },
	{ "**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL  },
	{ "**.htc", "text/x-component", NULL, NULL, do_file, NULL  },

	{ "general.js",  "text/javascript", no_cache_IE9, NULL, do_ej, do_auth },

	{ "**.js",  "text/javascript", no_cache_IE9, NULL, do_ej, do_auth },
	{ "**.cab", "text/txt", NULL, NULL, do_file, do_auth },
	{ "**.CFG", "application/force-download", NULL, NULL, do_prf_file, do_auth },

	{ "apply.cgi*", "text/html", no_cache_IE9, do_html_post_and_get, do_apply_cgi, do_auth },
	{ "upgrade.cgi*", "text/html", no_cache_IE9, do_upgrade_post, do_upgrade_cgi, do_auth},
	{ "upload.cgi*", "text/html", no_cache_IE9, do_upload_post, do_upload_cgi, do_auth },
	{ "syslog.cgi*", "application/force-download", no_cache_IE9, do_html_post_and_get, do_log_cgi, do_auth },
	{ "update.cgi*", "text/javascript", no_cache_IE9, do_html_post_and_get, do_update_cgi, do_auth }, // jerry5 

	{ NULL, NULL, NULL, NULL, NULL, NULL }
};

static int ej_get_usb_share_list(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int first_pool, ret;

	disks_info = read_disk_data();
	if (!disks_info)
		return 0;

	ret = 0;
	first_pool = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 1) {
				if (first_pool == 1)
					first_pool = 0;
				else
					ret += websWrite(wp, ", ");
				
				ret += websWrite(wp, "[\"%s\",\"%s\",\"%s\"]", 
					follow_partition->device, follow_partition->mount_point, follow_partition->file_system);
			}
		}

	free_disk_data(disks_info);

	return ret;
}

static int ej_get_AiDisk_status(int eid, webs_t wp, int argc, char **argv)
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *follow_info;
	int sh_num;
	char **folder_list = NULL;
	int first_pool, first_folder, result, i;

	websWrite(wp, "function get_cifs_status() {\n");
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_samba"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_ftp_status() {\n");
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_ftp"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_share_management_status(protocol) {\n");
	websWrite(wp, "    if (protocol == \"cifs\")\n");
	websWrite(wp, "	return %d;\n", nvram_get_int("st_samba_mode"));
	websWrite(wp, "    else if (protocol == \"ftp\")\n");
	websWrite(wp, "	return %d;\n", nvram_get_int("st_ftp_mode"));
	websWrite(wp, "    else\n");
	websWrite(wp, "	return -1;\n");
	websWrite(wp, "}\n\n");

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		websWrite(wp, "function get_sharedfolder_in_pool(poolName) {}\n");
		return -1;
	}
	first_pool = 1;
	websWrite(wp, "function get_sharedfolder_in_pool(poolName) {\n");
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				websWrite(wp, "    ");

				if (first_pool == 1)
					first_pool = 0;
				else
					websWrite(wp, "else ");

				follow_info = rindex(follow_partition->mount_point, '/');
				websWrite(wp, "if (poolName == \"%s\") {\n", follow_info+1);
				websWrite(wp, "	return [");

				result = get_all_folder_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list);
				if (result < 0) {
					websWrite(wp, "];\n");
					websWrite(wp, "    }\n");
					free_2_dimension_list(&sh_num, &folder_list);
					continue;
				}

				first_folder = 1;
				for (i = 0; i < sh_num; ++i) {
					if (first_folder == 1)
						first_folder = 0;
					else
						websWrite(wp, ", ");
					websWrite(wp, "\"%s\"", folder_list[i]);
				}

				websWrite(wp, "];\n");
				websWrite(wp, "    }\n");
			}
		}

	websWrite(wp, "}\n\n");

	if (disks_info != NULL) {
		free_2_dimension_list(&sh_num, &folder_list);
		free_disk_data(disks_info);
	}

	return 0;
}

static int ej_get_all_accounts(int eid, webs_t wp, int argc, char **argv)
{
	int acc_num = 0;
	char **account_list = NULL;
	char *acc_mode = "ftp";
	int i, first;

	ejArgs(argc, argv, "%s", &acc_mode);

	first = 1;
	if (strcmp(acc_mode, "ftp") == 0) {
		first = 0;
		websWrite(wp, "\"%s\"", FTP_ANONYMOUS_USER);
	}

	get_account_list(&acc_num, &account_list);
	for (i = 0; i < acc_num; ++i) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");

		websWrite(wp, "\"%s\"", account_list[i]);
	}
	free_2_dimension_list(&acc_num, &account_list);
	
	return 0;
}

static void start_flash_usbled()
{
	doSystem("killall %s %s", "-SIGUSR1", "detect_link");
}

static void stop_flash_usbled()
{
	doSystem("killall %s %s", "-SIGUSR2", "detect_link");
}

static int ej_safely_remove_disk(int eid, webs_t wp, int argc, char_t **argv)
{
	int result;
	int port_num = 0;
	char *disk_port = websGetVar(wp, "port", "");
	char *disk_devn = websGetVar(wp, "devn", "");

	start_flash_usbled();

	if (atoi(disk_port) == 1)
		port_num = 1;
	else if (atoi(disk_port) == 2)
		port_num = 2;
	
	result = doSystem("/sbin/ejusb %d %s", port_num, disk_devn);
	if (result != 0) {
		show_error_msg("Action9");
		websWrite(wp, "<script>\n");
		websWrite(wp, "safely_remove_disk_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		clean_error_msg();
	} else {
		websWrite(wp, "<script>\n");
		websWrite(wp, "safely_remove_disk_success();\n");
		websWrite(wp, "</script>\n");
	}

	stop_flash_usbled();

	return 0;
}

int ej_get_permissions_of_account(int eid, webs_t wp, int argc, char **argv) {
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num = 0;
	char **account_list = NULL;
	char *acc_mode = "ftp";
	int samba_right, ftp_right;
	int result, anonym, i, j;
	int first_pool, first_account, first_folder;

	ejArgs(argc, argv, "%s", &acc_mode);

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		websWrite(wp, "function get_account_permissions_in_pool(account, pool) {return [];}\n");
		return -1;
	}

	get_account_list(&acc_num, &account_list);

	websWrite(wp, "function get_account_permissions_in_pool(account, pool) {\n");

	anonym = 0;
	if (strcmp(acc_mode, "ftp") == 0) {
		anonym = 1;
	}

	if ((acc_num + anonym) <= 0)
		websWrite(wp, "    return [];\n");

	first_account = 1;
	for (i = 0; i < (acc_num + anonym); ++i) {
		char *acc_value;
		websWrite(wp, "    ");
		if (first_account == 1)
			first_account = 0;
		else
			websWrite(wp, "else ");
		
		if (i < acc_num)
			acc_value = account_list[i];
		else
			acc_value = FTP_ANONYMOUS_USER;

		websWrite(wp, "if (account == \"%s\") {\n", acc_value);

		first_pool = 1;
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
					int sh_num = 0;
					char **folder_list = NULL;
					websWrite(wp, "	");
					if (first_pool == 1)
						first_pool = 0;
					else
						websWrite(wp, "else ");

					websWrite(wp, "if (pool == \"%s\") {\n", rindex(follow_partition->mount_point, '/')+1);

					websWrite(wp, "	    return [");

					result = get_all_folder_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list);
					if (result != 0) {
						websWrite(wp, "];\n");
						websWrite(wp, "	}\n");
						free_2_dimension_list(&sh_num, &folder_list);
						continue;
					}
					first_folder = 1;
					for (j = 0; j < sh_num; ++j) {
						samba_right = get_permission(acc_value,
												 follow_partition->mount_point,
												 folder_list[j],
												 "cifs");
						ftp_right = get_permission(acc_value,
												 follow_partition->mount_point,
												 folder_list[j],
												 "ftp");
						if (samba_right < 0 || samba_right > 3) {
//							samba_right = DEFAULT_SAMBA_RIGHT;	// J++
							samba_right = 0;	
						}

						if (ftp_right < 0 || ftp_right > 3) {
//							ftp_right = DEFAULT_FTP_RIGHT;		// J++
							ftp_right = 0;
						}

						if (first_folder == 1) {
							first_folder = 0;
							websWrite(wp, "[\"%s\", %d, %d]", folder_list[j], samba_right, ftp_right);
						}
						else
							websWrite(wp, "		    [\"%s\", %d, %d]", folder_list[j], samba_right, ftp_right);

						if (j != sh_num-1)
							websWrite(wp, ",\n");
					}
					websWrite(wp, "];\n");
					websWrite(wp, "	}\n");
					
					free_2_dimension_list(&sh_num, &folder_list);
				}
			}
		}

		websWrite(wp, "    }\n");
	}

	websWrite(wp, "}\n\n");

	free_2_dimension_list(&acc_num, &account_list);

	if (disks_info != NULL)
		free_disk_data(disks_info);

	return 0;
}


int ej_get_folder_tree(int eid, webs_t wp, int argc, char **argv) {
	char *layer_order = websGetVar(wp, "layer_order", ""), folder_code[1024];
	char *follow_info, *follow_info_end, backup;
	int layer = 0, first;
	int disk_count, partition_count, folder_count1, folder_count2;
	int disk_order = -1, partition_order = -1, folder_order = -1;
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *pool_mount_dir;
	DIR *dir1, *dir2;
	struct dirent *dp1, *dp2;
	char dir1_Path[4096], dir2_Path[4096];

	if (strlen(layer_order) <= 0) {
		printf("No input \"layer_order\"!\n");
		return -1;
	}

	follow_info = index(layer_order, '_');
	while (follow_info != NULL && *follow_info != 0) {
		++layer;

		++follow_info;
		if (*follow_info == 0)
			break;
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && isdigit(*follow_info_end))
			++follow_info_end;
		backup = *follow_info_end;
		*follow_info_end = 0;

		if (layer == 1)
			disk_order = atoi(follow_info);
		else if (layer == 2)
			partition_order = atoi(follow_info);
		else if (layer == 3)
			folder_order = atoi(follow_info);
		*follow_info_end = backup;

		if (layer == 3) {
			memset(folder_code, 0, 1024);
			strcpy(folder_code, follow_info);
		}

		follow_info = follow_info_end;
	}
	follow_info = folder_code;

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		printf("Can't read the information of disks.\n");
		return -1;
	}

	first = 1;
	disk_count = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next, ++disk_count) {
		if (layer == 0) { // get disks.
			partition_count = 0;
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count)
				;

			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "\"%s#%u#%u\"", follow_disk->tag, disk_count, partition_count);

			continue;
		}
		if (disk_count != disk_order)
			continue;

		partition_count = 0;
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count) {
			if (follow_partition->mount_point == NULL || strlen(follow_partition->mount_point) <= 0)
				continue;

			pool_mount_dir = rindex(follow_partition->mount_point, '/')+1;

			if (layer == 1) { // get pools.
				dir2 = opendir(follow_partition->mount_point);
				/* pool_mount_dir isn't file. */
				if (dir2 == NULL)
					continue;

				folder_count2 = 0;
				while ((dp2 = readdir(dir2)) != NULL) {
					if (dp2->d_name[0] == '.')
						continue;

					++folder_count2;
				}
				closedir(dir2);

				if (first == 1)
					first = 0;
				else
					websWrite(wp, ", ");

				websWrite(wp, "\"%s#%u#%u\"", pool_mount_dir, partition_count, folder_count2);

				continue;
			}
			if (partition_count != partition_order)
				continue;

			memset(dir1_Path, 0, 4096);
			sprintf(dir1_Path, "%s", follow_partition->mount_point);
			dir1 = opendir(dir1_Path);
			if (dir1 == NULL) {
				printf("Can't open the directory, %s.\n", dir1_Path);

				free_disk_data(disks_info);

				return -1;
			}

			folder_count1 = -1;
			while ((dp1 = readdir(dir1)) != NULL) {
				if (dp1->d_name[0] == '.')
					continue;

				++folder_count1;

				if (layer == 2) { // get L1's folders.
					memset(dir2_Path, 0, 4096);
					sprintf(dir2_Path, "%s/%s", dir1_Path, dp1->d_name);
					dir2 = opendir(dir2_Path);

					folder_count2 = 0;
					if (dir2 != NULL) {
						while ((dp2 = readdir(dir2)) != NULL) {
							if (dp2->d_name[0] == '.')
								continue;

							++folder_count2;
						}
						closedir(dir2);
					}
					if (first == 1)
						first = 0;
					else
						websWrite(wp, ", ");

					websWrite(wp, "\"%s#%u#%u\"", dp1->d_name, folder_count1, folder_count2);

					continue;
				}

				if (folder_count1 == folder_order)
					sprintf(dir1_Path, "%s/%s", dir1_Path, dp1->d_name);
			}
			closedir(dir1);
		}
	}
	free_disk_data(disks_info);
	layer -= 3;

	while (layer >= 0) {      // get Ln's folders.
		/* get the current folder_code and folder_order. */
		follow_info_end = index(follow_info, '_');
		if (follow_info_end != NULL)
			follow_info = follow_info_end+1;
		else
			backup = -1;
		folder_order = atoi(follow_info);

		dir1 = opendir(dir1_Path);
		if (dir1 == NULL) {
			printf("Can't open the directory, %s.\n", dir1_Path);

			return -1;
		}
		folder_count1 = -1;
		while ((dp1 = readdir(dir1)) != NULL) {
			if (dp1->d_name[0] == '.')
				continue;

			++folder_count1;

			if (layer == 0) {
				memset(dir2_Path, 0, 4096);
				sprintf(dir2_Path, "%s/%s", dir1_Path, dp1->d_name);
				dir2 = opendir(dir2_Path);
				folder_count2 = 0;
				if (dir2 != NULL) {
					while ((dp2 = readdir(dir2)) != NULL) {
						if (dp2->d_name[0] == '.')
							continue;

						++folder_count2;
					}
					closedir(dir2);
				}

				if (first == 1)
					first = 0;
				else
					websWrite(wp, ", ");

				websWrite(wp, "\"%s#%u#%u\"", dp1->d_name, folder_count1, folder_count2);

				continue;
			}

			if (folder_count1 == folder_order)
				sprintf(dir1_Path, "%s/%s", dir1_Path, dp1->d_name);
		}
		closedir(dir1);
		--layer;
	}

	return 0;
}

int ej_get_share_tree(int eid, webs_t wp, int argc, char **argv) {
	char *layer_order = websGetVar(wp, "layer_order", "");
	char *follow_info, *follow_info_end, backup;
	int layer = 0, first;
	int disk_count, partition_count, share_count;
	int disk_order = -1, partition_order = -1;
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;

	if (strlen(layer_order) <= 0) {
		printf("No input \"layer_order\"!\n");
		return -1;
	}

	follow_info = index(layer_order, '_');
	while (follow_info != NULL && *follow_info != 0) {
		++layer;
		++follow_info;
		if (*follow_info == 0)
			break;
		follow_info_end = follow_info;
		while (*follow_info_end != 0 && isdigit(*follow_info_end))
			++follow_info_end;
		backup = *follow_info_end;
		*follow_info_end = 0;

		if (layer == 1)
			disk_order = atoi(follow_info);
		else if (layer == 2)
			partition_order = atoi(follow_info);
		else {
			*follow_info_end = backup;
			printf("Input \"%s\" is incorrect!\n", layer_order);
			return -1;
		}

		*follow_info_end = backup;
		follow_info = follow_info_end;
	}

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		printf("Can't read the information of disks.\n");
		return -1;
	}

	first = 1;
	disk_count = 0;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next, ++disk_count) {
		partition_count = 0;
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next, ++partition_count) {
			if (layer != 0 && follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				int i;
				char **folder_list;
				int result;
				result = get_all_folder_in_mount_path(follow_partition->mount_point, &share_count, &folder_list);
				if (result < 0) {
					printf("get_disk_tree: Can't get the folder list in \"%s\".\n", follow_partition->mount_point);

					share_count = 0;
				}

				if (layer == 2 && partition_count == partition_order && disk_count == disk_order) {
					for (i = 0; i < share_count; ++i) {
						if (first == 1)
							first = 0;
						else
							websWrite(wp, ", ");

						websWrite(wp, "\"%s#%u#0\"", folder_list[i], i);
					}
				}
				else if (layer == 1 && disk_count == disk_order) {
					if (first == 1)
						first = 0;
					else
						websWrite(wp, ", ");

					follow_info = rindex(follow_partition->mount_point, '/');
					websWrite(wp, "\"%s#%u#%u\"", follow_info+1, partition_count, share_count);
				}

				free_2_dimension_list(&share_count, &folder_list);
			}
		}
		if (layer == 0) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "\"%s#%u#%u\"", follow_disk->tag, disk_count, partition_count);
		}

		if (layer > 0 && disk_count == disk_order)
			break;
	}

	free_disk_data(disks_info);

	return 0;
}

void not_ej_initial_folder_var_file()						// J++
{
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;

	disks_info = read_disk_data();
	if (disks_info == NULL)
		return;

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next)
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				initial_folder_list_in_mount_path(follow_partition->mount_point);
//				initial_all_var_file_in_mount_path(follow_partition->mount_point);
			}

	free_disk_data(disks_info);
}

int ej_initial_folder_var_file(int eid, webs_t wp, int argc, char **argv)	// J++
{
//	not_ej_initial_folder_var_file();
	return 0;
}

int ej_set_share_mode(int eid, webs_t wp, int argc, char **argv) {
	int samba_mode = nvram_get_int("st_samba_mode");
	int ftp_mode = nvram_get_int("st_ftp_mode");
	char *dummyShareway = websGetVar(wp, "dummyShareway", "");
	char *protocol = websGetVar(wp, "protocol", "");
	char *mode = websGetVar(wp, "mode", "");
	int result;

	if (strlen(dummyShareway) > 0)
		nvram_set_temp("dummyShareway", dummyShareway);
	else
		nvram_set_int_temp("dummyShareway", 0);

	if (strlen(protocol) <= 0) {
		show_error_msg("Input1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(mode) <= 0) {
		show_error_msg("Input3");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (!strcmp(mode, "share")) {
		if (!strcmp(protocol, "cifs")) {
			if (samba_mode == 1 || samba_mode == 3)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_samba_mode", 1);
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 1)
				goto SET_SHARE_MODE_SUCCESS;
			nvram_set_int("st_ftp_mode", 1);
		}
		else {
			show_error_msg("Input2");

			websWrite(wp, "<script>\n");
			websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
			websWrite(wp, "</script>\n");

			clean_error_msg();
			return -1;
		}
	}
	else if (!strcmp(mode, "account")) {
		if (!strcmp(protocol, "cifs")) {
			if (samba_mode == 2 || samba_mode == 4)
				goto SET_SHARE_MODE_SUCCESS;

			nvram_set_int("st_samba_mode", 4);
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 2)
				goto SET_SHARE_MODE_SUCCESS;

			nvram_set_int("st_ftp_mode", 2);
		}
		else {
			show_error_msg("Input2");

			websWrite(wp, "<script>\n");
			websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
			websWrite(wp, "</script>\n");

			clean_error_msg();
			return -1;
		}
	}
	else if (!strcmp(mode, "anonym") && !strcmp(protocol, "ftp")) {
		if (ftp_mode == 3)
			goto SET_SHARE_MODE_SUCCESS;
		nvram_set_int("st_ftp_mode", 3);
	}
	else if (!strcmp(mode, "account_anonym") && !strcmp(protocol, "ftp")) {
		if (ftp_mode == 4)
			goto SET_SHARE_MODE_SUCCESS;
		nvram_set_int("st_ftp_mode", 4);
	}
	else {
		show_error_msg("Input4");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	nvram_commit_safe();

	not_ej_initial_folder_var_file();	// J++

	if (!strcmp(protocol, "cifs"))
		result = eval("/sbin/run_samba");
	else if (!strcmp(protocol, "ftp"))
		result = eval("/sbin/run_ftp");
	else {
		show_error_msg("Input2");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (result != 0) {
		show_error_msg("Action8");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_share_mode_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

SET_SHARE_MODE_SUCCESS:
	websWrite(wp, "<script>\n");
	websWrite(wp, "set_share_mode_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}


int ej_modify_sharedfolder(int eid, webs_t wp, int argc, char **argv) {
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *new_folder = websGetVar(wp, "new_folder", "");
	char *mount_path;

	//printf("[httpd] mod share folder\n");	// tmp test
	if (strlen(pool) <= 0) {
		show_error_msg("Input7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(folder) <= 0) {
		show_error_msg("Input9");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(new_folder) <= 0) {
		show_error_msg("Input17");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (get_mount_path(pool, &mount_path) < 0) {
		show_error_msg("System1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (mod_folder(mount_path, folder, new_folder) < 0) {
		show_error_msg("Action7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	free(mount_path);

	if (eval("/sbin/run_ftpsamba") != 0) {
		show_error_msg("Action7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "modify_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

int ej_delete_sharedfolder(int eid, webs_t wp, int argc, char **argv) {
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *mount_path;

	if (strlen(pool) <= 0) {
		show_error_msg("Input7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(folder) <= 0) {
		show_error_msg("Input9");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (get_mount_path(pool, &mount_path) < 0) {
		show_error_msg("System1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (del_folder(mount_path, folder) < 0) {
		show_error_msg("Action6");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	free(mount_path);

	if (eval("/sbin/run_ftpsamba") != 0) {
		show_error_msg("Action6");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "delete_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

int ej_create_sharedfolder(int eid, webs_t wp, int argc, char **argv) {
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *mount_path;

	printf("[httpd] create share folder\n");	// tmp test
	if (strlen(pool) <= 0) {
		show_error_msg("Input7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(folder) <= 0) {
		show_error_msg("Input9");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (get_mount_path(pool, &mount_path) < 0) {
		fprintf(stderr, "Can't get the mount_path of %s.\n", pool);

		show_error_msg("System1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (add_folder(mount_path, folder) < 0) {
		show_error_msg("Action5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	free(mount_path);

	system("nvram set chk=1");	// tmp test
	if (eval("/sbin/run_samba") != 0) {
		show_error_msg("Action5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_sharedfolder_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "create_sharedfolder_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

int ej_set_AiDisk_status(int eid, webs_t wp, int argc, char **argv) {
	char *protocol = websGetVar(wp, "protocol", "");
	char *flag = websGetVar(wp, "flag", "");
	int result = 0;

	printf("[httpd] set aidisk status\n");	// tmp test
	if (strlen(protocol) <= 0) {
		show_error_msg("Input1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(flag) <= 0) {
		show_error_msg("Input18");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (!strcmp(protocol, "cifs")) {
		if (!strcmp(flag, "on")) {
			nvram_set_int("enable_samba", 1);
			nvram_commit_safe();
			result = system("/sbin/run_samba");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set_int("enable_samba", 0);
			nvram_commit_safe();
			if (!pids("smbd"))
				goto SET_AIDISK_STATUS_SUCCESS;

			result = system("/sbin/stop_samba");
		}
		else {
			show_error_msg("Input19");

			websWrite(wp, "<script>\n");
			websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
			websWrite(wp, "</script>\n");

			clean_error_msg();
			return -1;
		}
	}
	else if (!strcmp(protocol, "ftp")) {
		if (!strcmp(flag, "on")) {
			nvram_set_int("enable_ftp", 1);
			nvram_commit_safe();
			result = system("run_ftp");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set_int("enable_ftp", 0);
			nvram_commit_safe();
			if (!pids("vsftpd"))
				goto SET_AIDISK_STATUS_SUCCESS;

			result = system("/sbin/stop_ftp");
		}
		else {
			show_error_msg("Input19");

			websWrite(wp, "<script>\n");
			websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
			websWrite(wp, "</script>\n");

			clean_error_msg();
			return -1;
		}
	}
	else {
		show_error_msg("Input2");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (result != 0) {
		show_error_msg("Action8");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_AiDisk_status_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

SET_AIDISK_STATUS_SUCCESS:
	websWrite(wp, "<script>\n");
	//websWrite(wp, "set_AiDisk_status_success();\n");
	websWrite(wp, "parent.resultOfSwitchAppStatus();\n");
	websWrite(wp, "</script>\n");

	printf("set aidisk done\n");	// tmp test
	return 0;
}

int ej_modify_account(int eid, webs_t wp, int argc, char **argv) {
	char *account = websGetVar(wp, "account", "");
	char *new_account = websGetVar(wp, "new_account", "");
	char *new_password = websGetVar(wp, "new_password", "");

	if (strlen(account) <= 0) {
		show_error_msg("Input5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(new_account) <= 0 && strlen(new_password) <= 0) {
		show_error_msg("Input16");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (mod_account(account, new_account, new_password) < 0) {
		show_error_msg("Action4");

		websWrite(wp, "<script>\n");
		websWrite(wp, "modify_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "modify_account_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

int ej_delete_account(int eid, webs_t wp, int argc, char **argv) {
	char *account = websGetVar(wp, "account", "");

	printf("[httpd] delete account\n");	// tmp test
	if (strlen(account) <= 0) {
		show_error_msg("Input5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	not_ej_initial_folder_var_file();	// J++

	if (del_account(account) < 0) {
		show_error_msg("Action3");

		websWrite(wp, "<script>\n");
		websWrite(wp, "delete_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "delete_account_success();\n");
	websWrite(wp, "</script>\n");
	return 0;
}

int ej_initial_account(int eid, webs_t wp, int argc, char **argv) {
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *command;
	int len, result;

	printf("[httpd] initial account\n");	// tmp test
	nvram_set_int("acc_num", 0);
	nvram_commit_safe();

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		show_error_msg("System2");

		websWrite(wp, "<script>\n");
		websWrite(wp, "initial_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next)
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				len = strlen("rm -f ")+strlen(follow_partition->mount_point)+strlen("/.__*");
				command = (char *)malloc(sizeof(char)*(len+1));
				if (command == NULL) {
					show_error_msg("System1");

					websWrite(wp, "<script>\n");
					websWrite(wp, "initial_account_error(\'%s\');\n", error_msg);
					websWrite(wp, "</script>\n");

					clean_error_msg();
					return -1;
				}
				sprintf(command, "rm -f %s/.__*", follow_partition->mount_point);
				command[len] = 0;

				result = system(command);
				free(command);

				initial_folder_list_in_mount_path(follow_partition->mount_point);
				initial_all_var_file_in_mount_path(follow_partition->mount_point);
			}

	free_disk_data(disks_info);

	if (eval("/sbin/run_ftpsamba") != 0) {
		show_error_msg("System1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "initial_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "initial_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int ej_create_account(int eid, webs_t wp, int argc, char **argv) {
	char *account = websGetVar(wp, "account", "");
	char *password = websGetVar(wp, "password", "");

	printf("[httpd] create account\n");	// tmp test
	if (strlen(account) <= 0) {
		show_error_msg("Input5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (strlen(password) <= 0) {
		show_error_msg("Input14");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	not_ej_initial_folder_var_file();	// J++

	if (add_account(account, password) < 0) {
		show_error_msg("Action2");

		websWrite(wp, "<script>\n");
		websWrite(wp, "create_account_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "create_account_success();\n");
	websWrite(wp, "</script>\n");

	return 0;
}

int ej_set_account_permission(int eid, webs_t wp, int argc, char **argv) {
	char *mount_path;
	char *account = websGetVar(wp, "account", "");
	char *pool = websGetVar(wp, "pool", "");
	char *folder = websGetVar(wp, "folder", "");
	char *protocol = websGetVar(wp, "protocol", "");
	char *permission = websGetVar(wp, "permission", "");
	int right;

	if (strlen(account) <= 0) {
		show_error_msg("Input5");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (test_if_exist_account(account) != 1) {
		show_error_msg("Input6");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (strlen(pool) <= 0) {
		show_error_msg("Input7");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}
	if (get_mount_path(pool, &mount_path) < 0) {
		fprintf(stderr, "Can't get the mount_path of %s.\n", pool);

		show_error_msg("System1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");

		clean_error_msg();
		return -1;
	}

	if (strlen(folder) <= 0) {
		show_error_msg("Input9");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	if (strlen(protocol) <= 0) {
		show_error_msg("Input1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	if (strcmp(protocol, "cifs") && strcmp(protocol, "ftp") && strcmp(protocol, "dms")) {
		show_error_msg("Input2");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}

	if (strlen(permission) <= 0) {
		show_error_msg("Input12");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}
	right = atoi(permission);
	if (right < 0 || right > 3) {
		show_error_msg("Input13");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}

	if (set_permission(account, mount_path, folder, protocol, right) < 0) {
		show_error_msg("Action1");

		websWrite(wp, "<script>\n");
		websWrite(wp, "set_account_permission_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		free(mount_path);

		clean_error_msg();
		return -1;
	}

	websWrite(wp, "<script>\n");
	websWrite(wp, "set_account_permission_success();\n");
	websWrite(wp, "</script>\n");
	free(mount_path);
	return 0;
}

// traffic monitor
static int 
ej_netdev(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE * fp;
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
ej_bandwidth(int eid, webs_t wp, int argc, char_t **argv)
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
ej_backup_nvram(int eid, webs_t wp, int argc, char_t **argv)
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
// end svg support by Viz ^^^^^^^^^^^^^^^^^^^^

static int
ej_select_list(int eid, webs_t wp, int argc, char_t **argv)
{
	char *id;
	int ret = 0;	
	char out[64], idxstr[12], tmpstr[64], tmpstr1[64];
	int i, curr, hit, noneFlag;
	char *ref1, *ref2, *refnum;

	if (ejArgs(argc, argv, "%s", &id) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if (strcmp(id, "Storage_x_SharedPath")==0)
	{
		ref1 = "sh_path_x";
		ref2 = "sh_path";
		refnum = "sh_num";
		curr = atoi(nvram_get(ref1));
		sprintf(idxstr, "%d", curr);
		strcpy(tmpstr1, nvram_get(strcat_r(ref2, idxstr, tmpstr)));
		sprintf(out, "%s", tmpstr1);
		ret += websWrite(wp, out);
		return ret;
	}
	else if (strncmp(id, "Storage_x_AccUser", 17)==0)
	{
		sprintf(tmpstr, "sh_accuser_x%s", id + 17);
		ref2 = "acc_username";
		refnum = "acc_num";

		curr = atoi(nvram_get(tmpstr));
		noneFlag =1;
	}
	else if (strcmp(id, "Storage_x_AccAny")==0)
	{
		
		 ret = websWrite(wp, "<option value=\"Guest\">Guest</option>");
		 return ret;
	}
	else if (strcmp(id, "Storage_AllUser_List")==0)
	{

		strcpy(out, "<option value=\"Guest\">Guest</option>");
		ret += websWrite(wp, out);

		for (i=0;i<atoi(nvram_get("acc_num"));i++)
		{
			 sprintf(idxstr, "%d", i);
			 strcpy(tmpstr1, nvram_get(strcat_r("acc_username", idxstr, tmpstr)));
			 sprintf(out, "<option value=\"%s\">%s</option>", tmpstr1, tmpstr1);
			 ret += websWrite(wp, out);
		}
		return ret;
	}
	else 
	{
		 return ret;     
	}
	
	hit = 0;
 
	for (i=0;i<atoi(nvram_get(refnum));i++)
	{     
 		 sprintf(idxstr, "%d", i);
		 strcpy(tmpstr1, nvram_get(strcat_r(ref2, idxstr, tmpstr)));
	     	 sprintf(out, "<option value=\"%d\"", i);

		 if (i==curr) 
		 {
			hit = 1;
			sprintf(out, "%s selected", out);
		 }
		 sprintf(out,"%s>%s</option>", out, tmpstr1);       
 
		 ret += websWrite(wp, out);
	}     

	if (noneFlag)
	{
		cprintf("hit : %d\n", hit);
		if (!hit) sprintf(out, "<option value=\"99\" selected>None</option>");
		else sprintf(out, "<option value=\"99\">None</option>");

		ret += websWrite(wp, out);
	}	
	return ret;
}

struct ej_handler ej_handlers[] = {
	{ "nvram_get_x", ej_nvram_get_x},
	{ "nvram_get_list_x", ej_nvram_get_list_x},
	{ "nvram_get_buf_x", ej_nvram_get_buf_x},
	{ "nvram_get_table_x", ej_nvram_get_table_x},
	{ "nvram_match_x", ej_nvram_match_x},
	{ "nvram_double_match_x", ej_nvram_double_match_x},
	{ "nvram_match_both_x", ej_nvram_match_both_x},
	{ "nvram_match_list_x", ej_nvram_match_list_x},
	{ "select_channel", ej_select_channel},
	{ "uptime", ej_uptime},
	{ "sysuptime", ej_sysuptime},
	{ "nvram_dump", ej_dump},
	{ "load_script", ej_load},
	{ "select_list", ej_select_list},
	{ "firmware_caps_hook", firmware_caps_hook},

//tomato qosvvvvvvvvvvv 2010.08 Viz
        { "netdev", ej_netdev},
        { "bandwidth", ej_bandwidth},
        { "nvram", ej_backup_nvram},
//tomato qos^^^^^^^^^^^^ end Viz

	{ "nvram_get_ddns", ej_nvram_get_ddns},
	{ "nvram_char_to_ascii", ej_nvram_char_to_ascii},
	{ "update_variables", update_variables_ex},
	{ "asus_nvram_commit", asus_nvram_commit},
	{ "notify_services", ej_notify_services},
	{ "detect_if_wan", detect_if_wan},
	{ "detect_dhcp_pppoe", detect_dhcp_pppoe},
	{ "wanlink", wanlink_hook},
	{ "lanlink", lanlink_hook},
	{ "wan_action", wan_action_hook},
	{ "wol_action", wol_action_hook},
	{ "nvram_action", nvram_action_hook},
	{ "nf_values", nf_values_hook},
	{ "get_parameter", ej_get_parameter},
	{ "login_state_hook", login_state_hook},
	{ "get_nvram_list", ej_get_nvram_list},
	{ "dhcp_leases", ej_dhcp_leases},
	{ "get_arp_table", ej_get_arp_table},
	{ "get_static_client", ej_get_static_client},
	{ "get_static_ccount", ej_get_static_ccount},
	{ "get_vpns_client", ej_get_vpns_client},
	{ "wl_auth_list", ej_wl_auth_list},
	{ "wl_scan_5g", ej_wl_scan_5g},
	{ "wl_scan_2g", ej_wl_scan_2g},
	{ "wl_bssid_5g", ej_wl_bssid_5g},
	{ "wl_bssid_2g", ej_wl_bssid_2g},
	{ "ej_system_status", ej_system_status_hook},
	{ "shown_language_option", ej_shown_language_option},
	{ "disk_pool_mapping_info", ej_disk_pool_mapping_info},
	{ "available_disk_names_and_sizes", ej_available_disk_names_and_sizes},
	{ "get_usb_ports_info", ej_get_usb_ports_info},
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
	{ "create_account", ej_create_account},	/*no ccc*/
	{ "delete_account", ej_delete_account}, /*n*/
	{ "modify_account", ej_modify_account}, /*n*/
	{ "create_sharedfolder", ej_create_sharedfolder},	/*y*/
	{ "delete_sharedfolder", ej_delete_sharedfolder},	/*y*/
	{ "modify_sharedfolder", ej_modify_sharedfolder},	/* no ccc*/
	{ "set_share_mode", ej_set_share_mode},
	{ "initial_folder_var_file", ej_initial_folder_var_file},	/* J++ */
	{ "vpnc_state_hook", vpnc_state_hook},
	{ "openvpn_srv_cert_hook", openvpn_srv_cert_hook},
	{ "openvpn_cli_cert_hook", openvpn_cli_cert_hook},
	{ NULL, NULL }
};

