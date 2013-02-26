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
/*
 * ASUS Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ASUSTeK Inc..
 *
 * $Id: web_ex.c,v 1.4 2007/04/09 12:01:50 shinjung Exp $
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

#include "httpd.h"

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/reboot.h>
#include <net/ethernet.h>   //add by Viz 2010.08

#include <nvram/bcmnvram.h>
#include <shutils.h>
#include "common.h"

#include "nvram_x.h"

#include <disk_io_tools.h>
#include <disk_initial.h>
#include <disk_share.h>
#include "initial_web_hook.h"

#include <linux/types.h>
#include <net/if.h>
#include <linux/sockios.h>

#include <wireless.h>

#include <ralink.h>
#include <boards.h>
#include <notify_rc.h>
#include <linux/rtl8367_ioctl.h>

#include <sys/mman.h>

#define MAX_GROUP_ITEM 10
#define MAX_GROUP_COUNT 64
#define MAX_LINE_SIZE 512

#ifndef O_BINARY
#define O_BINARY	0
#endif

#include <image.h>
#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

#include <syslog.h>
#define logs(s) syslog(LOG_NOTICE, s)

#define wan_prefix(unit, prefix)	snprintf(prefix, sizeof(prefix), "wan%d_", unit)

static void nvram_commit_safe()
{
	nvram_commit();
}

#define sys_upload(image) eval("nvram", "restore", image)
#define sys_download(file) eval("nvram", "save", file)

#define GROUP_FLAG_REFRESH 	0
#define GROUP_FLAG_DELETE 	1
#define GROUP_FLAG_ADD 		2
#define GROUP_FLAG_REMOVE 	3

#define IMAGE_HEADER		"HDR0"
#define PROFILE_HEADER		"HDR1"
#define PROFILE_HEADER_NEW	"HDR2"
#define IH_MAGIC		0x27051956	/* Image Magic Number */
#define FW_MTD_NAME		"Firmware_Stub"

static int apply_cgi_group(webs_t wp, int sid, struct variable *var, char *groupName, int flag);
static int nvram_generate_table(webs_t wp, char *serviceId, char *groupName);

int count_sddev_mountpoint();

static unsigned long restart_needed_bits = 0;
static unsigned int restart_total_time = 0; 

#define ACTION_UPGRADE_OK   0
#define ACTION_UPGRADE_FAIL 1

int action;
char *serviceId;
char *groupItem[MAX_GROUP_ITEM];
char urlcache[128];
char *next_host;
int delMap[MAX_GROUP_COUNT+2];
char SystemCmd[128];
char UserID[32]="";
char UserPass[32]="";
char ProductID[32]="";
extern int redirect;
extern int change_passwd;	// 2008.08 magic
extern int reget_passwd;	// 2008.08 magic

void
sys_reboot()
{
	dbG("[httpd] reboot...\n");

	nvram_set("reboot", "1");
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

void websRedirect(webs_t wp, char_t *url)
{	
	websWrite(wp, T("<html><head>\r\n"));
	websWrite(wp, T("<meta http-equiv=\"refresh\" content=\"0; url=http://%s/%s\">\r\n"), getip((FILE *)wp), url);
	websWrite(wp, T("<meta http-equiv=\"Content-Type\" content=\"text/html\">\r\n"));
	websWrite(wp, T("</head></html>\r\n"));      
	
	websDone(wp, 200);
}
*/
void websRedirect(webs_t wp, const char *url)
{
	websWrite(wp, "<html><head>\r\n");
	if (next_host && *next_host)
		websWrite(wp, "<meta http-equiv=\"refresh\" content=\"0; url=http://%s/%s\">\r\n", next_host, url);
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
		
		nvram_set("mfp_ip_requeue", s_addr);
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
		
		nvram_set("mfp_ip_monopoly", s_addr);
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
	else if (strcmp(name,"eject-usb.sh")==0)
	{
		eval("rmstorage");
	}
	else if (strcmp(name,"ddnsclient")==0)
	{
		eval("start_ddns");
	}
	else if (strcmp(name,"hostname_check") == 0)
	{
		notify_rc("manual_ddns_hostname_check");
	}
	else if (strstr(scmd, " ") == 0) // no parameter, run script with eval
	{
		eval(scmd);
	}
	else
		system(scmd);
}

void websScan(char_t *str)
{
	unsigned int i, flag;
	char_t *v1, *v2, *v3, *sp;
	char_t groupid[64];
	char_t value[MAX_LINE_SIZE];
	char_t name[MAX_LINE_SIZE];
	
	v1 = strchr(str, '?');
	
	i = 0;
	flag = 0;
	
	while (v1!=NULL)
	{	   	    	
	    v2 = strchr(v1+1, '=');
	    v3 = strchr(v1+1, '&');

		if (v2 == NULL)
			break;
	    
	    if (v3!=NULL)
	    {
	       strncpy(value, v2+1, v3-v2-1);
	       value[v3-v2-1] = 0;  
	    }  
	    else
	    {
	       strcpy(value, v2+1);
	    }
	    
	    strncpy(name, v1+1, v2-v1-1);
	    name[v2-v1-1] = 0;
	    
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
		   sprintf(groupid, "%s_s", value);
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
#ifdef TRANSLATE_ON_FLY
	do_ej (url, wp);
	websDone (wp, 200);
#else   // define TRANSLATE_ON_FLY

     FILE *fp;
     char buf[MAX_LINE_SIZE];

     fp = fopen(url, "r");
     
     if (fp==NULL) return;
     
     while (fgets(buf, sizeof(buf), fp))
     {
	websWrite(wp, buf);
     } 
     
     websDone(wp, 200);	
     fclose(fp);
#endif
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
	char *sid, *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint(*c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	return ret;
}

static int
ej_nvram_get_ddns(int eid, webs_t wp, int argc, char_t **argv)
{
	char *sid, *name, *c;
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &sid, &name) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}

	for (c = nvram_safe_get(name); *c; c++) {
		if (isprint(*c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

	if (strcmp(name,"ddns_return_code")==0) {
		if (!nvram_match("ddns_return_code", "ddns_query")) {
			nvram_set("ddns_return_code","");
		}
	}

	return ret;
}

/*
 * Example: 
 * lan_ipaddr=192.168.1.1
 * <% nvram_get_x("lan_ipaddr"); %> produces "192.168.1.1"
 * <% nvram_get_x("undefined"); %> produces ""
 */
static int
ej_nvram_get_f(int eid, webs_t wp, int argc, char_t **argv)
{
	char *file, *field, *c, buf[64];
	int ret = 0;

	if (ejArgs(argc, argv, "%s %s", &file, &field) < 2) {
		websError(wp, 400, "Insufficient args\n");
		return -1;
	}
	
	strcpy(buf, nvram_safe_get(field));
	for (c = buf; *c; c++) {
		if (isprint(*c) &&
		    *c != '"' && *c != '&' && *c != '<' && *c != '>')
			ret += websWrite(wp, "%c", *c);
		else
			ret += websWrite(wp, "&#%d", *c);
	}

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

static int
ej_urlcache(int eid, webs_t wp, int argc, char_t **argv)
{
	int flag;
		
	if (strcmp(urlcache, "Main_Operation.asp")==0)
	   flag = 2;
	else if (strcmp(urlcache, "Main_Content.asp")==0)
	   flag = 1;
	else
	   flag = 0;
	   
	cprintf("Url:%s %d\n", urlcache, flag);

	if (strcmp(nvram_safe_get("wan_route_x"), "IP_Routed")==0)
	{	
	   if (strcmp(nvram_safe_get("wan_nat_x"), "1")==0)
	   {	   
	   	/* disable to redirect page */
	   	if (redirect)
	   	{	
			websWrite(wp, "Basic_GOperation_Content.asp");
			redirect=0;
	   	}
	   	else if (flag==2)
			websWrite(wp, "Basic_GOperation_Content.asp");
	   	else if (flag==1)
			websWrite(wp, "Basic_HomeGateway_SaveRestart.asp");
	   	else
	   		websWrite(wp, "Main_Index_HomeGateway.asp");
	    }
	    else
	    {	
	   	/* disable to redirect page */
	   	if (redirect)
	   	{	
			websWrite(wp, "Basic_ROperation_Content.asp");
			redirect=0;
	   	}
	   	else if (flag==2)
			websWrite(wp, "Basic_ROperation_Content.asp");
	   	else if (flag==1)
			websWrite(wp, "Basic_Router_SaveRestart.asp");
	   	else
	   		websWrite(wp, "Main_Index_Router.asp");
	    }	
	}      
	else
	{
	    if (flag==2)
		websWrite(wp, "Basic_AOperation_Content.asp");
	    else if (flag==1)	
		websWrite(wp, "Basic_AccessPoint_SaveRestart.asp");
	    else	
	    	websWrite(wp, "Main_Index_AccessPoint.asp");
	}
	strcpy(urlcache,"");
	
	return 0;
}


/* Report sys up time */
static int
ej_uptime(int eid, webs_t wp, int argc, char_t **argv)
{
	char buf[MAX_LINE_SIZE];
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
	char buf[MAX_LINE_SIZE];
	int ret = 0;

	fp = fopen(filename, "r");
	if (fp==NULL) 
	{
		ret+=websWrite(wp, "%s", "");
		return (ret);
	}

	while (fgets(buf, MAX_LINE_SIZE, fp)!=NULL)
	{
	    ret += websWrite(wp, buf);
	}

	fclose(fp);

	return (ret);
}

static int
ej_dump(int eid, webs_t wp, int argc, char_t **argv)
{	
	char filename[32];
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
	else if (strcmp(file, "leases.log")==0) 
		return (ej_lan_leases(eid, wp, 0, NULL));
	else if (strcmp(file, "vpns_list.log")==0) 
		return (ej_vpns_leases(eid, wp, 0, NULL));
	else if (strcmp(file, "iptable.log")==0) 
		return (ej_nat_table(eid, wp, 0, NULL));
	else if (strcmp(file, "route.log")==0)
		return (ej_route_table(eid, wp, 0, NULL));
	ret = 0;
	
	if (strcmp(file, "syslog.log")==0)
	{
		sprintf(filename, "/tmp/%s-1", file);
		ret+=dump_file(wp, filename); 
	}
	
	sprintf(filename, "/tmp/%s", file);
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
validate_cgi(webs_t wp, int sid, int groupFlag)
{
    struct variable *v;    
    char *value;
    char name[64];
	
    /* Validate and set variables in table order */
    for (v = GetVariables(sid); v->name != NULL; v++) 
    {
	memset(name, 0, 64); //2008.08 magic
	sprintf(name, "%s", v->name);
	
	if ((value = websGetVar(wp, name, NULL)))
	{
		if (strcmp(v->longname, "Group"))
		{
			nvram_set(v->name, value);
		}
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

//2008.08 magic {
extern char *read_whole_file(const char *target) {
	FILE *fp = fopen(target, "r");
	char *buffer, *new_str;
	int i;
	unsigned int read_bytes = 0;
	unsigned int each_size = 1024;
	
	if (fp == NULL)
		return NULL;
	
	buffer = (char *)malloc(sizeof(char)*each_size+read_bytes);
	if (buffer == NULL) {
		csprintf("No memory \"buffer\".\n");
		fclose(fp);
		return NULL;
	}
	memset(buffer, 0, sizeof(char)*each_size+read_bytes);
	
	while ((i = fread(buffer+read_bytes, sizeof(char), each_size, fp)) == each_size) {
		read_bytes += each_size;
		new_str = (char *)malloc(sizeof(char)*each_size+read_bytes);
		if (new_str == NULL) {
			csprintf("No memory \"new_str\".\n");
			free(buffer);
			fclose(fp);
			return NULL;
		}
		memset(new_str, 0, sizeof(char)*each_size+read_bytes);
		memcpy(new_str, buffer, read_bytes);
		
		free(buffer);
		buffer = new_str;
	}
	
	fclose(fp);
	return buffer;
}

static char post_buf[10000] = { 0 };
static char post_buf_backup[10000] = { 0 };

static void do_html_post_and_get(char *url, FILE *stream, int len, char *boundary) {
	char *query = NULL;
	
	init_cgi(NULL);
	
	memset(post_buf, 0, 10000);
	memset(post_buf_backup, 0, 10000);
	
	if (fgets(post_buf, MIN(len+1, sizeof(post_buf)), stream)) {
		len -= strlen(post_buf);
		
		while (len--)
			(void)fgetc(stream);
	}
	
	query = url;
	strsep(&query, "?");
	
	if (query && strlen(query) > 0) {
		if (strlen(post_buf) > 0)
			sprintf(post_buf_backup, "?%s&%s", post_buf, query);
		else
			sprintf(post_buf_backup, "?%s", query);
		
		sprintf(post_buf, "%s", post_buf_backup+1);
	}
	else if (strlen(post_buf) > 0)
		sprintf(post_buf_backup, "?%s", post_buf);
	
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

int nvram_modified = 0;
int wl_modified = 0;
int rt_modified = 0;

void set_wifi_ssid(char* ifname, char* value)
{
	doSystem("iwpriv %s set SSID=\"%s\"", ifname, value);
}

void set_wifi_param_int(char* ifname, char* param, char* value, int val_min, int val_max)
{
	int i_value = atoi(value);
	if (i_value < val_min) i_value = val_min;
	if (i_value > val_max) i_value = val_max;

	doSystem("iwpriv %s set %s=%d", ifname, param, i_value);
}

void set_wifi_mrate(char* ifname, char* value)
{
	int i_value = atoi(value);
	int mphy = 3;
	int mmcs = 1;

	switch (i_value)
	{
	case 0: // HTMIX (1S) 6.5-15 Mbps
		mphy = 3;
		mmcs = 0;
		break;
	case 1: // HTMIX (1S) 15-30 Mbps
		mphy = 3;
		mmcs = 1;
		break;
	case 2: // HTMIX (1S) 19.5-45 Mbps
		mphy = 3;
		mmcs = 2;
		break;
	case 3: // HTMIX (2S) 13-30 Mbps
		mphy = 3;
		mmcs = 8;
		break;
	case 4: // HTMIX (2S) 26-60 Mbps
		mphy = 3;
		mmcs = 9;
		break;
	case 5: // OFDM 9 Mbps
		mphy = 2;
		mmcs = 1;
		break;
	case 6: // OFDM 12 Mbps
		mphy = 2;
		mmcs = 2;
		break;
	case 7: // OFDM 18 Mbps
		mphy = 2;
		mmcs = 3;
		break;
	case 8: // OFDM 24 Mbps
		mphy = 2;
		mmcs = 4;
		break;
	case 9: // CCK 11 Mbps
		mphy = 1;
		mmcs = 3;
		break;
	}

	doSystem("iwpriv %s set McastPhyMode=%d", ifname, mphy);
	doSystem("iwpriv %s set McastMcs=%d", ifname, mmcs);
}


static int validate_asp_apply(webs_t wp, int sid, int groupFlag) {
	struct variable *v;
	char *value;
	char name[64];
	char buff[100];
	
	/* Validate and set variables in table order */
	for (v = GetVariables(sid); v->name != NULL; ++v) {
		memset(name, 0, sizeof(name));
		sprintf(name, "%s", v->name);

		if ((value = websGetVar(wp, name, NULL))) {

			if (!strcmp(v->longname, "Group")) {
				;
			} else if (!strcmp(v->name, "wl_country_code")) {
				
				if ( (strcmp(nvram_safe_get(name), value)) && (doSystem("/sbin/setCountryCode %s", value) == 0) ) {
				
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
				
				if (!strncmp(v->name, "wl_", 3) && strcmp(v->name, "wl_ssid2"))
				{
					if (!strcmp(v->name, "wl_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("wl_ssid2", buff);
						set_wifi_ssid(WIF, value);
					}
					else if (!strcmp(v->name, "wl_guest_ssid"))
					{
						set_wifi_ssid("ra1", value);
					}
					else if (!strcmp(v->name, "wl_TxPower"))
					{
						set_wifi_param_int(WIF, "TxPower", value, 0, 100);
					}
					else if (!strcmp(v->name, "wl_greenap"))
					{
						set_wifi_param_int(WIF, "GreenAP", value, 0, 1);
					}
					else if (!strcmp(v->name, "wl_IgmpSnEnable"))
					{
						set_wifi_param_int(WIF, "IgmpSnEnable", value, 0, 1);
					}
					else if (!strcmp(v->name, "wl_mcastrate"))
					{
						set_wifi_mrate(WIF, value);
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
				
				if (!strncmp(v->name, "rt_", 3) && strcmp(v->name, "rt_ssid2"))
				{
					if (!strcmp(v->name, "rt_ssid"))
					{
						memset(buff, 0, sizeof(buff));
						char_to_ascii(buff, value);
						nvram_set("rt_ssid2", buff);
						set_wifi_ssid(WIF2G, value);
					}
					else if (!strcmp(v->name, "rt_guest_ssid"))
					{
						set_wifi_ssid("rai1", value);
					}
					else if (!strcmp(v->name, "rt_TxPower"))
					{
						set_wifi_param_int(WIF2G, "TxPower", value, 0, 100);
					}
					else if (!strcmp(v->name, "rt_greenap"))
					{
						set_wifi_param_int(WIF2G, "GreenAP", value, 0, 1);
					}
					else if (!strcmp(v->name, "rt_IgmpSnEnable"))
					{
						set_wifi_param_int(WIF2G, "IgmpSnEnable", value, 0, 1);
					}
					else if (!strcmp(v->name, "rt_mcastrate"))
					{
						set_wifi_mrate(WIF2G, value);
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

	return nvram_modified;
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
	char groupId[64];
	char *groupName;
	int result;

	restart_needed_bits = 0;

	// assign control variables
	action_mode = websGetVar(wp, "action_mode", "");
	script = websGetVar(wp, "action_script", "");
	sid_list = websGetVar(wp, "sid_list", "");

	csprintf("Apply: [%s] [%s]\n", action_mode, script); //2009.01 magic for debug

	while ((serviceId = svc_pop_list(sid_list, ';'))) {
		sid = 0;
		while (GetServiceId(sid) != NULL) {
			if (!strcmp(GetServiceId(sid), serviceId))
				break;
			
			sid++;
		}
		
		if (serviceId != NULL) {
			if (!strcmp(action_mode, "  Save  ") || !strcmp(action_mode, " Apply ")) {
				if (!validate_asp_apply(wp, sid, TRUE)) {
					websWrite(wp, "<script>no_changes_and_no_committing();</script>\n");
				}
				else {
					nvram_set("x_Setting", "1");
					nvram_set("w_Setting", "1");	// J++
					websWrite(wp, "<script>done_committing();</script>\n");
				}
			}
			else if (!strcmp(action_mode, "Update")) {
				validate_asp_apply(wp, sid, TRUE);
			}
			else {
				strcpy(groupId, websGetVar(wp, "group_id", ""));
				
				if (strlen(action_mode) > 0) {
					groupName = groupId;
					if (!strcmp(action_mode, " Add ")) {
						result = apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_ADD);
						
						if (result == 1)
							nvram_set("page_modified", "1");
						
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
					else if (!strcmp(action_mode, " Del ")) {
						result = apply_cgi_group(wp, sid, NULL, groupName, GROUP_FLAG_REMOVE);
						
						if (result == 1)
							nvram_set("page_modified", "1");
						
						websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
					}
					else if (!strcmp(action_mode, " Restart ")) {
						struct variable *v;
						
						for (v = GetVariables(sid); v->name != NULL; ++v) {
						if (!strcmp(v->name, groupName))
							break;
					}
					
					printf("--- Restart group %s. ---\n", groupName);
					dbG("debug v->event: 0x%x\n", v->event);
					restart_needed_bits |= v->event;
					dbG("debug restart_needed_bits: 0x%lx\n", restart_needed_bits);
					
					if (!strcmp(groupName, "RBRList") || !strcmp(groupName, "ACLList"))
						wl_modified |= WIFI_COMMON_CHANGE_BIT;
					if (!strcmp(groupName, "rt_RBRList") || !strcmp(groupName, "rt_ACLList"))
						rt_modified |= WIFI_COMMON_CHANGE_BIT;
						
						validate_asp_apply(wp, sid, FALSE);	// for some nvram with this group
						
						nvram_set("page_modified", "0");
						nvram_set("x_Setting", "1");
						nvram_set("w_Setting", "1");	// J++
						
						if (!strcmp(script, "goonsetting")) {
							websWrite(wp, "<script>done_validating(\"%s\");</script>\n", action_mode);
							script = "";
						}
						else {
							websWrite(wp, "<script>done_committing();</script>\n");
						}
					}
					
					validate_cgi(wp, sid, FALSE);	// for some nvram with this group group
				}
			}
		}

		sid_list = sid_list+strlen(serviceId)+1;
	}

	if (strlen(script) > 0) {
		if (!strcmp(script, "networkmap_refresh")) {
			eval("restart_networkmap");
			websWrite(wp, "<script>restart_needed_time(1);</script>\n");
		}
		else if (!strcmp(script, "mfp_monopolize"))
		{
			sys_script(script);
			websWrite(wp, "<script>restart_needed_time(3);</script>\n");
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
		if ((restart_needed_bits & RESTART_IPV6) != 0)
			restart_total_time = MAX(ITVL_RESTART_IPV6, restart_total_time);
		if ((restart_needed_bits & RESTART_LAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_LAN, restart_total_time);
		if ((restart_needed_bits & RESTART_DHCPD) != 0)
			restart_total_time = MAX(ITVL_RESTART_DHCPD, restart_total_time);
		if ((restart_needed_bits & RESTART_WAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_WAN, restart_total_time);
		if ((restart_needed_bits & RESTART_IPTV) != 0)
			restart_total_time = MAX(ITVL_RESTART_IPTV, restart_total_time);
		if ((restart_needed_bits & RESTART_FTPSAMBA) != 0)
			restart_total_time = MAX(ITVL_RESTART_FTPSAMBA, restart_total_time);
		if ((restart_needed_bits & RESTART_TERMINAL) != 0)
			restart_total_time = MAX(ITVL_RESTART_TERMINAL, restart_total_time);
		if ((restart_needed_bits & RESTART_VPNSRV) != 0)
			restart_total_time = MAX(ITVL_RESTART_VPNSRV, restart_total_time);
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
		if ((restart_needed_bits & RESTART_QOS) != 0)
			restart_total_time = MAX(ITVL_RESTART_QOS, restart_total_time);
		if ((restart_needed_bits & RESTART_SWITCH) != 0)
			restart_total_time = MAX(ITVL_RESTART_SWITCH, restart_total_time);
		if ((restart_needed_bits & RESTART_SWITCH_VLAN) != 0)
			restart_total_time = MAX(ITVL_RESTART_SWITCH_VLAN, restart_total_time);
		if ((restart_needed_bits & RESTART_SYSLOG) != 0)
			restart_total_time = MAX(ITVL_RESTART_SYSLOG, restart_total_time);
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
				if (wl_modified & WIFI_COMMON_CHANGE_BIT)
					restart_total_time = MAX(ITVL_RESTART_WIFI, restart_total_time);
				else
					restart_total_time = MAX(1, restart_total_time);
			}
			else if (rt_modified) {
				if (rt_modified & WIFI_COMMON_CHANGE_BIT)
					restart_total_time = MAX(ITVL_RESTART_WIFI_INIC, restart_total_time);
				else
					restart_total_time = MAX(1, restart_total_time);
			}
		}
		
		websWrite(wp, "<script>restart_needed_time(%d);</script>\n", restart_total_time);
	}
	
	return 0;
}

static int asus_nvram_commit(int eid, webs_t wp, int argc, char_t **argv) {
	if (restart_needed_bits != 0 || nvram_modified == 1) {
		nvram_modified = 0;
#ifndef NVRAM_NOCOMMIT
		nvram_commit_safe();
#endif
	}
	
	return 0;
}

extern long uptime(void);

static int ej_notify_services(int eid, webs_t wp, int argc, char_t **argv) {
	restart_total_time = 0;
	if (restart_needed_bits != 0) {
		if ((restart_needed_bits & RESTART_REBOOT) != 0) {
			printf("*** Run notify_rc restart_reboot! \n");
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
				restart_needed_bits &= ~(u32)RESTART_VPNSRV;		// vpn server already re-started (RESTART_LAN)
				restart_needed_bits &= ~(u32)RESTART_FIREWALL;		// firewall already re-started (RESTART_LAN)
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
				restart_needed_bits &= ~(u32)ITVL_RESTART_IPTV;
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
			if ((restart_needed_bits & RESTART_VPNSRV) != 0) {
				notify_rc("restart_vpn_server");
				restart_needed_bits &= ~(u32)RESTART_VPNSRV;
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
			if ((restart_needed_bits & RESTART_DMS) != 0) {
				notify_rc("restart_dms");
				restart_needed_bits &= ~(u32)RESTART_DMS;
			}
			if ((restart_needed_bits & RESTART_TORRENT) != 0) {
				notify_rc("restart_torrent");
				restart_needed_bits &= ~(u32)RESTART_TORRENT;
			}
			if ((restart_needed_bits & RESTART_ARIA) != 0) {
				notify_rc("restart_aria");
				restart_needed_bits &= ~(u32)RESTART_ARIA;
			}
			if ((restart_needed_bits & RESTART_ITUNES) != 0) {
				notify_rc("restart_itunes");
				restart_needed_bits &= ~(u32)RESTART_ITUNES;
			}
			if ((restart_needed_bits & RESTART_QOS) != 0) {
				notify_rc("restart_qos");
				restart_needed_bits &= ~(u32)RESTART_QOS;
			}
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
							nvram_set("reload_svc_wl", "1");
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
							nvram_set("reload_svc_rt", "1");
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

// for error_page.htm's detection
static int detect_if_wan(int eid, webs_t wp, int argc, char_t **argv) {
	int if_wan = is_phyconnected();
	
	websWrite(wp, "%d", if_wan);
	
	return 0;
}

char *Ch_conv(char *proto_name, int idx)
{
        char *proto;
        char qos_name_x[32];
        sprintf(qos_name_x, "%s%d", proto_name, idx);
        if (nvram_match(qos_name_x,""))
        {
                return NULL;
        }
        else
        {
                proto=nvram_get(qos_name_x);
                return proto;
        }
}

// Define of return value: 1st bit is NTP, 2nd bit is WAN DNS, 3rd bit is more open DNS.
static int detect_wan_connection(int eid, webs_t wp, int argc, char_t **argv) {
	int MAX_LOOKUP_NUM = 1, lookup_num;
	//int got_ntp = 0, got_ping = 0;
	int result = 0;
	FILE *fp;
	char buf[128], word[16], *next;
	char *ping_cmd[] = {"ping", word, "-c", "1", NULL};
	char *dns_list = NULL;
	int i;
	char *MORE_DNS = "8.8.8.8 208.67.220.220 8.8.4.4 208.67.222.222";
	
	memset(buf, 0, 128);
	
	for(lookup_num = 0; lookup_num < MAX_LOOKUP_NUM; ++lookup_num) {
		if (nvram_match("ntp_ready", "1"))
			//got_ntp = 1;
			result += 1;

		if (nvram_match("wan_proto", "static"))
			dns_list = nvram_safe_get("wan_dns_t");
		else
			dns_list = nvram_safe_get("wan0_dns");

		foreach(word, dns_list, next) {
			dbg("Try to ping dns: %s...\n", word);
			_eval(ping_cmd, ">/tmp/log.txt", 0, NULL);
			
			if ((fp = fopen("/tmp/log.txt", "r")) == NULL)
				continue;
			
			for(i = 0; i < 2 && fgets(buf, 128, fp) != NULL; ++i) {
				dbg("%d. Got the results: %s.\n", i+1, buf);
				if (strstr(buf, "alive") || strstr(buf, " ms"))
					//got_ping = 1;
					result += 2;
				
				//if (got_ping)
				if (result >= 2)
					break;
			}
			fclose(fp);
			
			//if (got_ping)
			if (result >= 2)
				break;
		}

		dbg("Try to check more dns: %s...\n", MORE_DNS);
		int dns_test = 0;
		foreach(word, MORE_DNS, next) {
			if (dns_test == 0 && !strcmp(nvram_safe_get("dns_test"), "1")) {
				dns_test = 1;
				continue;
			}
			dbg("Try to ping dns: %s...\n", word);
			
			doSystem("/usr/sbin/tcpcheck 4 %s:53 >/tmp/log.txt", word);
			
			if ((fp = fopen("/tmp/log.txt", "r")) == NULL)
				continue;
			
			for(i = 0; i < 2 && fgets(buf, 128, fp) != NULL; ++i) {
				dbg("%d. Got the results: %s.\n", i+1, buf);
				if (strstr(buf, "alive")/* || strstr(buf, " ms")*/)
					result += 4;
				
				if (result >= 4)
					break;
			}
			fclose(fp);
			
			//if (got_ping)
			if (result >= 4)
				break;
		}
		
		if (result > 0) {
			websWrite(wp, "%d", result);
			break;
		}
		else if (lookup_num == MAX_LOOKUP_NUM-1) {
			dbg("Can't get the host from ntp or response from DNS!\n");
			websWrite(wp, "-1");
			break;
		}
	}
	
	return 0;
}

void logmessage(char *logheader, char *fmt, ...)
{
  va_list args;
  char buf[512];

  va_start(args, fmt);

  vsnprintf(buf, sizeof(buf), fmt, args);
  openlog(logheader, 0, 0);
  syslog(0, buf);
  closelog();
  va_end(args);
}

static int detect_dhcp_pppoe(int eid, webs_t wp, int argc, char_t **argv) {
	if (nvram_match("wan_nat_x", "1"))       // Gateway mode
		;
	else if (nvram_match("wan_route_x", "IP_Routed"))	// Router mode
		;
	else {   // AP mode
		websWrite(wp, "AP mode");
		return 0;
	}

	if(get_usb_modem_state())
		websWrite(wp, "modem");
	else
		websWrite(wp, "dhcp");

	return 0;
}

static int get_wan_status_log(int eid, webs_t wp, int argc, char_t **argv) {
	FILE *fp = fopen("/tmp/wanstatus.log", "r");
	char log_info[64];
	int i;
	
	memset(log_info, 0, 64);
	
	if (fp != NULL) {
		fgets(log_info, 64, fp);
		
		i = 0;
		while (log_info[i] != 0) {
			if (log_info[i] == '\n') {
				log_info[i] = 0;
				break;
			}
			
			++i;
		}
		
		websWrite(wp, "%s", log_info);
		fclose(fp);
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

void fill_switch_port_status(int ioctl_id, char linkstate[32])
{
	int fd;
	int link_value = -1;
	char *link_duplex;
	
	fd = open(RTL8367_DEVPATH, O_RDONLY);
	if (fd > 0)
	{
		if (ioctl(fd, ioctl_id, &link_value) < 0)
			link_value = -1;
		
		close(fd);
	}
	
	if (link_value >= 0)
	{
		if (link_value & 0x010000)
		{
			if (link_value & 0x0100)
			{
				link_duplex = "Full Duplex";
			}
			else
			{
				link_duplex = "Half Duplex";
			}
			
			switch (link_value & 0x03)
			{
			case 2:
				link_value = 1000;
				break;
			case 1:
				link_value = 100;
				break;
			default:
				link_value = 10;
				break;
			}
			
			sprintf(linkstate, "%d Mbps, %s", link_value, link_duplex);
		}
		else
		{
			sprintf(linkstate, "No link");
		}
	}
	else
	{
		sprintf(linkstate, "SMI I/O Error");
	}
}

int
get_if_status(const char *wan_ifname)
{
	int s, status;
	struct ifreq ifr;
	struct sockaddr_in *wan_addr_in;
	
	if (nvram_match("wan_route_x", "IP_Bridged"))
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
	char type[32], dns[256], dns_item[80], statusstr[32], etherlink[32] = {0};
	int status = 0, unit, is_first, i_wan_src_phy, i_ioctl_id;
	long ppp_time;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	char *wan0_ip, *wanx_ip, *wan0_gw, *wanx_gw, *wan_ip6, *lan_ip6;
#if defined (USE_IPV6)
	int ipv6_type;
	char *wan6_ifname = NULL;
	char addr6_wan[INET6_ADDRSTRLEN], addr6_lan[INET6_ADDRSTRLEN];
#endif
	
	wanx_ip = "";
	wanx_gw = "";
	wan_ip6 = "";
	lan_ip6 = "";
	ppp_time = 0;
	
	/* current unit */
	if ((unit = nvram_get_int("wan_unit")) < 0)
		unit = 0;
	
	wan_prefix(unit, prefix);
	
	statusstr[0] = 0;
	
	if(get_usb_modem_state())
	{
		if(nvram_match("modem_type", "3"))
			status = get_if_status(nvram_safe_get("rndis_ifname"));
		else {
#if defined (USE_IPV6)
			if (nvram_get_int("ip6_wan_if") == 0)
				wan6_ifname = IFNAME_PPP;
#endif
			status = get_if_status(IFNAME_PPP);
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
	if (!is_phyconnected()) {
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
	
	if(get_usb_modem_state())
	{
		if(nvram_match("modem_type", "3"))
			strcpy(type, "Modem (RNDIS)");
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
			if (*dns_item) {
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
	if (i_wan_src_phy == 4)
		i_ioctl_id = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN4;
	else if (i_wan_src_phy == 3)
		i_ioctl_id = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN3;
	else if (i_wan_src_phy == 2)
		i_ioctl_id = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN2;
	else if (i_wan_src_phy == 1)
		i_ioctl_id = RTL8367_IOCTL_STATUS_SPEED_PORT_LAN1;
	else
		i_ioctl_id = RTL8367_IOCTL_STATUS_SPEED_PORT_WAN;
	
	fill_switch_port_status(i_ioctl_id, etherlink);
	
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

	return 0;
}

static int lanlink_hook(int eid, webs_t wp, int argc, char_t **argv) 
{
	char etherlink1[32] = {0};
	char etherlink2[32] = {0};
	char etherlink3[32] = {0};
	char etherlink4[32] = {0};
	char etherlink5[32] = {0};

	fill_switch_port_status(RTL8367_IOCTL_STATUS_SPEED_PORT_WAN,  etherlink1);
	fill_switch_port_status(RTL8367_IOCTL_STATUS_SPEED_PORT_LAN1, etherlink2);
	fill_switch_port_status(RTL8367_IOCTL_STATUS_SPEED_PORT_LAN2, etherlink3);
	fill_switch_port_status(RTL8367_IOCTL_STATUS_SPEED_PORT_LAN3, etherlink4);
	fill_switch_port_status(RTL8367_IOCTL_STATUS_SPEED_PORT_LAN4, etherlink5);

	websWrite(wp, "function lanlink_etherlink_wan()  { return '%s';}\n", etherlink1);
	websWrite(wp, "function lanlink_etherlink_lan1() { return '%s';}\n", etherlink2);
	websWrite(wp, "function lanlink_etherlink_lan2() { return '%s';}\n", etherlink3);
	websWrite(wp, "function lanlink_etherlink_lan3() { return '%s';}\n", etherlink4);
	websWrite(wp, "function lanlink_etherlink_lan4() { return '%s';}\n", etherlink5);

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
		needed_seconds = 5;
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
		sys_result = doSystem("/usr/bin/ether-wake -i %s %s", IFNAME_BR, wol_mac);
	
	if (sys_result == 0) 
	{
		nvram_set("wol_mac_last", wol_mac);
		websWrite(wp, "{\"wol_mac\": \"%s\"}", wol_mac);
	}
	else
		websWrite(wp, "{\"wol_mac\": \"null\"}");
	
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

static int usb_apps_check_hook(int eid, webs_t wp, int argc, char_t **argv) 
{
#if defined(APP_XUPNPD)
	int found_app_xupnpd = 1;
#else
	int found_app_xupnpd = 0;
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
	websWrite(wp, "function found_app_xupnpd() { return %d;}\n", found_app_xupnpd);
	websWrite(wp, "function found_app_dlna() { return %d;}\n", found_app_dlna);
	websWrite(wp, "function found_app_ffly() { return %d;}\n", found_app_ffly);
	websWrite(wp, "function found_app_torr() { return %d;}\n", found_app_trmd);
	websWrite(wp, "function found_app_aria() { return %d;}\n", found_app_aria);
	
	return 0;
}

static int board_caps_hook(int eid, webs_t wp, int argc, char_t **argv) 
{
#if defined(LED_ALL)
	int has_led_all = 1;
#else
	int has_led_all = 0;
#endif
#if defined(BTN_WPS)
	int has_but_wps = 1;
#else
	int has_but_wps = 0;
#endif
#if defined(USE_RT3352_MII)
	int has_inic_mii = 1;
#else
	int has_inic_mii = 0;
#endif
#if defined(USE_RTL8367_IGMP_SNOOPING)
	int has_switch_igmp = 1;
#else
	int has_switch_igmp = 0;
#endif
	websWrite(wp, "function support_but_wps() { return %d;}\n", has_but_wps);
	websWrite(wp, "function support_led_all() { return %d;}\n", has_led_all);
	websWrite(wp, "function support_led_phy() { return %d;}\n", ETH_PHY_LEDS);
	websWrite(wp, "function support_switch_igmp() { return %d;}\n", has_switch_igmp);
	websWrite(wp, "function support_apcli_only() { return %d;}\n", (has_inic_mii) ? 0 : 1);
	websWrite(wp, "function support_wl_stream_tx() { return %d;}\n", RT3883_RF_TX);
	websWrite(wp, "function support_wl_stream_rx() { return %d;}\n", RT3883_RF_RX);

	return 0;
}

static int kernel_caps_hook(int eid, webs_t wp, int argc, char_t **argv) 
{
#if defined(USE_IPV6)
	int has_ipv6 = 1;
#else
	int has_ipv6 = 0;
#endif
#if defined(USE_IPV6_HW_NAT)
	int has_ipv6_ppe = 1;
#else
	int has_ipv6_ppe = 0;
#endif
#if defined(USE_KERNEL3X)
	int wive_rtnl = 0;
#else
	int wive_rtnl = 1;
#endif
#if defined(USE_RT3352_MII)
	int min_vlan_ext = 4;
#else
	int min_vlan_ext = 3;
#endif
	websWrite(wp, "function support_ipv6() { return %d;}\n", has_ipv6);
	websWrite(wp, "function support_ipv6_ppe() { return %d;}\n", has_ipv6_ppe);
	websWrite(wp, "function support_min_vlan() { return %d;}\n", min_vlan_ext);
	websWrite(wp, "function support_ppp_policer() { return %d;}\n", wive_rtnl);
	websWrite(wp, "function support_fastnat() { return %d;}\n", wive_rtnl);
	
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
	char buf[MAX_LINE_SIZE+MAX_LINE_SIZE];
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
				sprintf(buf, "\"%s\"", nvram_get_list_x(gv->name, i));
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
	
//	sys_script("leases.sh");
	
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
		
		websWrite(wp, "[");
		
		if (!strcmp(dh_host, "*"))
			dh_host[0] = 0;
		
		// cut hostname to 18 chars
		dh_host[18] = 0;
		
		// convert MAC to upper case
		for (i=0; i<strlen(dh_mac); i++)
			dh_mac[i] = toupper(dh_mac[i]);
		
		websWrite(wp, "\"%s\", ", dh_host);
		websWrite(wp, "\"%s\", ", dh_mac);
		websWrite(wp, "\"%s\", ", dh_ip);
		
		if (strcmp(dh_sid, "*") == 0)
			websWrite(wp, "\"Manual\"");
		else
			websWrite(wp, "\"%s\"", dh_lease);
		
		websWrite(wp, "]");
		
		buff[0] = 0;
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
	int len, i, first_client, first_field;
	
	fp = fopen("/tmp/static_ip.inf", "r");
	if (!fp) {
		return 0;
	}
	
	first_client = 1;
	buf[0] = 0;
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
				memset(field, 0, 1024);
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
		
		buf[0] = 0;
	}
	
	fclose(fp);
	
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


static int ej_get_changed_status(int eid, webs_t wp, int argc, char_t **argv) {
	char *arp_info = read_whole_file("/proc/net/arp");
	char *disk_info = read_whole_file(PARTITION_FILE); 
	char *mount_info = read_whole_file("/proc/mounts"); 
	u32 arp_info_len, disk_info_len, mount_info_len; 
//	u32 arp_change, disk_change;
	
	//printf("get changed status\n");	// tmp test

	if (arp_info != NULL) {
		arp_info_len = strlen(arp_info);
		free(arp_info);
	}
	else
		arp_info_len = 0;
	
	if (disk_info != NULL) {
		disk_info_len = strlen(disk_info);
		free(disk_info);
	}
	else
		disk_info_len = 0;
	
	if (mount_info != NULL) {
		mount_info_len = strlen(mount_info);
		free(mount_info);
	}
	else
		mount_info_len = 0;

	websWrite(wp, "function get_client_status_changed() {\n");
	websWrite(wp, "    return %u;\n", arp_info_len);
	websWrite(wp, "}\n\n");
	
	websWrite(wp, "function get_disk_status_changed() {\n");
	websWrite(wp, "    return %u;\n", disk_info_len);
	websWrite(wp, "}\n\n");
	
	websWrite(wp, "function get_mount_status_changed() {\n");
	websWrite(wp, "    return %u;\n", mount_info_len);
	websWrite(wp, "}\n\n");
	
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
		st->radio = (nvram_get_int("mlme_radio_wl")) ? 1 : 0;
		if (st->radio)
			st->ap_guest = is_interface_up("ra1");
		else
			st->ap_guest = 0;
	}
	else
	{
		st->radio = (nvram_get_int("mlme_radio_rt")) ? 1 : 0;
		if (st->radio)
			st->ap_guest = is_interface_up("rai1");
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

	websWrite(wp, "function total_disk_sizes() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");

		websWrite(wp, "\"%llu\"", follow_disk->size_in_kilobytes);
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function disk_interface_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");

		websWrite(wp, "\"usb\"");
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
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

	websWrite(wp, "function pool_mirror_counts() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "0");
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

	websWrite(wp, "function pool_kilobytes() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "%llu", follow_partition->size_in_kilobytes);
		}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	websWrite(wp, "function pool_encryption_password_is_missing() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next)
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (first == 1)
				first = 0;
			else
				websWrite(wp, ", ");

			websWrite(wp, "\"no\"");
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

	u64 disk_used_kilobytes;

	websWrite(wp, "function disk_usage() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");

		disk_used_kilobytes = 0;
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next)
			disk_used_kilobytes += follow_partition->size_in_kilobytes;

		websWrite(wp, "%llu", disk_used_kilobytes);
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

//					if (strcmp(follow_disk2->tag, follow_disk->tag) == 0)
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
	free_disk_data(&disks_info);

	return 0;
}

static int ej_available_disk_names_and_sizes(int eid, webs_t wp, int argc, char_t **argv) {
	disk_info_t *disks_info, *follow_disk;
	int first;

	websWrite(wp, "function available_disks() { return [];}\n\n");
	websWrite(wp, "function available_disk_sizes() { return [];}\n\n");
	websWrite(wp, "function claimed_disks() { return [];}\n\n");
	websWrite(wp, "function claimed_disk_interface_names() { return [];}\n\n");
	websWrite(wp, "function claimed_disk_model_info() { return [];}\n\n");
	websWrite(wp, "function claimed_disk_total_size() { return [];}\n\n");
	websWrite(wp, "function claimed_disk_total_mounted_number() { return [];}\n\n");
	websWrite(wp, "function blank_disks() { return [];}\n\n");
	websWrite(wp, "function blank_disk_interface_names() { return [];}\n\n");
	websWrite(wp, "function blank_disk_model_info() { return [];}\n\n");
	websWrite(wp, "function blank_disk_total_size() { return [];}\n\n");
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
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

	/* show interface of the foreign disks */
	websWrite(wp, "function foreign_disk_interface_names() {\n");
	websWrite(wp, "    return [");
	first = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		if (first == 1)
			first = 0;
		else
			websWrite(wp, ", ");

//		websWrite(wp, "\"USB\"");
		websWrite(wp, "\"%s\"", follow_disk->port);
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

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
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");

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
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
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
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
	free_disk_data(&disks_info);

	return 0;
}

#if 0
static int ej_get_printer_info(int eid, webs_t wp, int argc, char_t **argv) {
	FILE *lpfp;
	char manufacturer[100], models[100], serialnos[100], pool[100];
	char buf[500];
	char *lpparam, *value, *v1 = NULL;
	
	if (!(lpfp = fopen("/proc/usblp/usblpid", "r"))) {
		strcpy(manufacturer, "");
		strcpy(models, "");
		strcpy(serialnos, "");
		strcpy(pool, "");
	}
	else {
		while (fgets(buf, 500, lpfp)) {
			value = &buf[0];
			lpparam = strsep(&value, "=")?:&buf[0];
			
			if (value) {
				v1 = strchr(value, '\n');
				*v1 = '\0';
				
				if (!strcmp(lpparam, "Manufacturer"))
					sprintf(manufacturer, "\"%s\"", value);
				else if (!strcmp(lpparam, "Model"))
					sprintf(models, "\"%s\"", value );
			}
		}
		
		/* compatible for WL700gE platform */
		strcpy(pool, "VirtualPool");
		fclose(lpfp);
	}
	
	websWrite(wp, "function printer_manufacturers() {\n return [%s];\n}\n", manufacturer);
	websWrite(wp, "function printer_models() {\n return [%s];\n}\n", models);
	websWrite(wp, "function printer_serialn() {\n return [%s];\n}\n", "");
	websWrite(wp, "function printer_pool() {\n return [\"%s\"];\n}\n", pool);
}
#else
#define MAX_PRINTER_NUM 2

static int ej_get_printer_info(int eid, webs_t wp, int argc, char_t **argv){
	int port_num = 0, first;
	char tmp[64], prefix[] = "usb_pathX";
	char printer_array[2][5][64];
	for(port_num = 1; port_num <= MAX_PRINTER_NUM; ++port_num){
		snprintf(prefix, sizeof(prefix), "usb_path%d", port_num);
		memset(printer_array[port_num-1][0], 0, 64);
		memset(printer_array[port_num-1][1], 0, 64);
		memset(printer_array[port_num-1][2], 0, 64);
		memset(printer_array[port_num-1][3], 0, 64);
		memset(printer_array[port_num-1][4], 0, 64);
		if(nvram_match(prefix, "printer")){
			strncpy(printer_array[port_num-1][0], "printer", 64);
			strncpy(printer_array[port_num-1][1], nvram_safe_get(strcat_r(prefix, "_manufacturer", tmp)), 64);
			strncpy(printer_array[port_num-1][2], nvram_safe_get(strcat_r(prefix, "_product", tmp)), 64);
			strncpy(printer_array[port_num-1][3], nvram_safe_get(strcat_r(prefix, "_serial", tmp)), 64);
			strcpy(printer_array[port_num-1][4], "VirtualPool");
		}
	}
	websWrite(wp, "function printer_manufacturers(){\n");
	websWrite(wp, "    return [");
	first = 1;
	for(port_num = 1; port_num <= MAX_PRINTER_NUM; ++port_num){
		if(strlen(printer_array[port_num-1][0]) > 0)
			websWrite(wp, "\"%s\"", printer_array[port_num-1][1]);
		else
			websWrite(wp, "\"\"");
		if(first){
			--first;
			websWrite(wp, ", ");
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
	websWrite(wp, "function printer_models(){\n");
	websWrite(wp, "    return [");
	first = 1;
	for(port_num = 1; port_num <= MAX_PRINTER_NUM; ++port_num){
		if(strlen(printer_array[port_num-1][0]) > 0)
			websWrite(wp, "\"%s\"", printer_array[port_num-1][2]);
		else
			websWrite(wp, "\"\"");
		if(first){
			--first;
			websWrite(wp, ", ");
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
	websWrite(wp, "function printer_serialn(){\n");
	websWrite(wp, "    return [");
	first = 1;
	for(port_num = 1; port_num <= MAX_PRINTER_NUM; ++port_num){
		if(strlen(printer_array[port_num-1][0]) > 0)
			websWrite(wp, "\"%s\"", printer_array[port_num-1][3]);
		else
			websWrite(wp, "\"\"");
		if(first){
			--first;
			websWrite(wp, ", ");
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
	websWrite(wp, "function printer_pool(){\n");
	websWrite(wp, "    return [");
	first = 1;
	for(port_num = 1; port_num <= MAX_PRINTER_NUM; ++port_num){
		if(strlen(printer_array[port_num-1][0]) > 0)
			websWrite(wp, "\"%s\"", printer_array[port_num-1][4]);
		else
			websWrite(wp, "\"\"");
		if(first){
			--first;
			websWrite(wp, ", ");
		}
	}
	websWrite(wp, "];\n");
	websWrite(wp, "}\n\n");
	return 0;
}
#endif

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

	//if (!query)
	//	goto footer;
	
	urlStr[0] = 0;
	
	value = websGetVar(wp, "action_mode","");
	next_host = websGetVar(wp, "next_host", "");
	current_url = websGetVar(wp, "current_page", "");
	next_url = websGetVar(wp, "next_page", "");
	script = websGetVar(wp, "action_script","");
	
	cprintf("Apply: %s %s %s %s\n", value, current_url, next_url, websGetVar(wp, "group_id", ""));
	
	if (!strcmp(value," Refresh "))
	{
		syscmd = websGetVar(wp,"SystemCmd","");
		strncpy(SystemCmd, syscmd, sizeof(SystemCmd)-1);
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value," Clear "))
	{
		// current only syslog implement this button
		unlink("/tmp/syslog.log-1");
		unlink("/tmp/syslog.log");
		websRedirect(wp, current_url);
		return 0;
	}
	else if (!strcmp(value,"NEXT"))
	{
		websRedirect(wp, next_url);
		return 0;
	}
	else if (!strcmp(value, "Save&Restart "))
	{
		dbG("[httpd] action mode: %s\n", value);

		websApply(wp, "Restarting.asp");
		nvram_set("x_Setting", "1");
		nvram_set("w_Setting", "1");	// J++
		eval("early_convert");
		nvram_commit_safe();

		sys_reboot();

		return (0);
	}
	else if (!strcmp(value, " Restart "))
	{
		dbG("[httpd] action mode: %s\n", value);

		websApply(wp, "Restarting.asp");

		sys_reboot();

		return (0);
	}
	else if (!strcmp(value, "Restore"))
	{
		dbG("[httpd] action mode: %s\n", value);

		websApply(wp, "Restarting.asp");
		eval("reset_to_defaults");

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
					validate_cgi(wp, sid, TRUE);
				}
				else if (!strcmp(value, "Set") || !strcmp(value, "Unset") || 
					 !strcmp(value, "Update") || !strcmp(value, "Eject") || !strcmp(value, "Check") )
				{
					validate_cgi(wp, sid, TRUE);
				}
				else if (!strcmp(value," Finish "))
				{
					validate_cgi(wp, sid, TRUE);
				}
				else
				{
					cprintf("group ?\n");
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

							sprintf(urlStr, "%s#%s", current_url, groupName);
							validate_cgi(wp, sid, FALSE);
						}
					}
				}
			}
			
			sid_list = sid_list+strlen(serviceId)+1;
		}
		
		//printf("apply????\n");
		
		/* Add for Internet Access Quick Setup */
		//special_handler(wp, url);
		
		/* Add for NVRAM mapping if necessary */
		//nvMap(current_url);
		
		/* Add for EMI Test page */
		if (strcmp(script, ""))
		{
			sys_script(script);
		}
		
		if (!strcmp(value, "  Save  ") || !strcmp(value, " Apply "))
		{
			strcpy(urlcache, next_url);
			websRedirect(wp, next_url);
		}
		else if (!strcmp(value, " Finish "))
			websRedirect(wp, "SaveRestart.asp");
		else if (urlStr[0] == 0)
			websRedirect(wp, current_url);
		else
			websRedirect(wp, urlStr);
		
		cprintf("apply ok\n");
		return 0;
	}
	
	return 1;
}
//2008.08 magic}

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
    char buf[MAX_LINE_SIZE+MAX_LINE_SIZE];
    char bufs[MAX_LINE_SIZE+MAX_LINE_SIZE];    
    int i, j, fieldLen, rowLen, fieldCount, value;
//    unsigned short int wstr[33];
    int addlen=0;
//    int temp=0;
    
//Yau
//printf("+++ add group table +++\n");
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
//    	printf("bufs len:  %d\n", strlen(bufs));
//    	printf("name: %s\n", gv->name);
//    	printf("buf:  %s\n", buf);
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
    
    if (strlen(bufs)> rowLen)
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
//Yau
//printf("=== generate_table === group:%s\n",groupName);     
   
   sid = LookupServiceId(serviceId);
	 
   if (sid==-1) return 0;
    
   /* Validate and set vairables in table order */	      
   for (v = GetVariables(sid); v->name != NULL; v++) 
   {
      /* printf("Find group : %s %s\n", v->name, groupName);*/
      if (!strcmp(groupName, v->name)) break;		       
   }    
   //Yau
//   printf("Find group :: %s %s\n", v->name, groupName);
   
   if (v->name == NULL) return 0;    
	    
   groupCount = nvram_get_int(v->argv[3]);
	    
//Yau
//printf("groupCount=%d\n",groupCount);	    

   if (groupCount==0) ret = nvram_add_group_table(wp, serviceId, v, -1);
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
do_auth(char *userid, char *passwd, char *realm)
{
	if (strcmp(ProductID,"")==0)
	{
		strncpy(ProductID, nvram_safe_get("productid"), 32);
	}
	
	if (strcmp(UserID,"")==0 || reget_passwd)
	{
		strncpy(UserID, nvram_safe_get("http_username"), 32);
	}
	
	if (strcmp(UserPass, "")==0 || reget_passwd)
	{
		strncpy(UserPass, nvram_safe_get("http_passwd"), 32);
	}
	
	if (reget_passwd)
	{
		reget_passwd = 0;
	}
	
	strncpy(userid, UserID, AUTH_MAX);
	strncpy(passwd, UserPass, AUTH_MAX);
	strncpy(realm, ProductID, AUTH_MAX);
//	strcpy(passwd, "");
}

static void
do_apply_cgi(char *url, FILE *stream)
{
    apply_cgi(stream, NULL, NULL, 0, url, NULL, NULL);
}

//#if defined(ASUS_MIMO) && defined(TRANSLATE_ON_FLY)
#ifdef TRANSLATE_ON_FLY
static int refresh_title_asp = 0;

static void
do_lang_cgi(char *url, FILE *stream)
{
	if (refresh_title_asp)  {
		// Request refreshing pages from browser.
		websHeader(stream);
		websWrite(stream, "<head></head><title>REDIRECT TO INDEX.ASP</title>");

		// The text between <body> and </body> content may be rendered in Opera browser.
		websWrite(stream, "<body onLoad='if (navigator.appVersion.indexOf(\"Firefox\")!=-1||navigator.appName == \"Netscape\") {top.location=\"index.asp\";}else {top.location.reload(true);}'></body>");
		websFooter(stream);
		websDone(stream, 200);
	} else {
		// Send redirect-page if and only if refresh_title_asp is true.
		// If we do not send Title.asp, Firefox reload web-pages again and again.
		// This trick had been deprecated due to compatibility issue with Netscape and Mozilla browser.
		websRedirect(stream, "Title.asp");
	}
}


static void
do_lang_post(char *url, FILE *stream, int len, char *boundary)
{
	int c;
	char *p, *p1;
	char orig_lang[4], new_lang[4];
	char buf[1024];

	if (url == NULL)
		return;

	p = strstr (url, "preferred_lang_menu");
	if (p == NULL)
		return;
	memset (new_lang, 0, sizeof (new_lang));
	strncpy (new_lang, p + strlen ("preferred_lang_menu") + 1, 2);

	memset (orig_lang, 0, sizeof (orig_lang));
	p1 = nvram_safe_get("preferred_lang");
	if (p1[0] != '\0')      {
		strncpy (orig_lang, p1, 2);
	} else {
		strncpy (orig_lang, "EN", 2);
	}

	// read remain data
	while ((c = fread(buf, 1, sizeof(buf), stream)) > 0)
		;

	refresh_title_asp = 0;
	if (strcmp (orig_lang, new_lang) != 0 || is_firsttime ())       {
		// If language setting is different or first change language
		nvram_set("preferred_lang", new_lang);
		if (is_firsttime ())    {
			nvram_set("x_Setting", "1");
			nvram_set("w_Setting", "1");	// J++
		}
		refresh_title_asp = 1;
#ifndef NVRAM_NOCOMMIT
		nvram_commit_safe();
#endif
	}
}
//#endif  // defined(ASUS_MIMO) && defined(TRANSLATE_ON_FLY)
#endif // TRANSLATE_ON_FLY


static void
do_webcam_cgi(char *url, FILE *stream)
{
	#define MAX_RECORD_IMG 12
	char *query, *path;
	char pic[32];
	int query_idx, last_idx;
		
	query = url;
	path = strsep(&query, "?") ? : url;
	cprintf("WebCam CGI\n");
	//ret = fcntl(fileno(stream), F_GETOWN, 0);
	query_idx = atoi(query);
	last_idx = nvram_get_int("WebPic");
			
	if (query_idx==0) sprintf(pic, "../var/tmp/display.jpg");
	else sprintf(pic, "../var/tmp/record%d.jpg", (query_idx>last_idx+1) ? (last_idx+1+MAX_RECORD_IMG-query_idx):(last_idx+1-query_idx));
	
	cprintf("\nWebCam: %s\n", pic);
	do_file(pic, stream);
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
checkcrc (const char *fw_image)
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
		fprintf(stderr, "Can't open %s: %s\n", fw_image, strerror(errno));
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
		fprintf (stderr, "Can't stat %s: %s\n",
			fw_image, strerror(errno));
		ret=-1;
		goto checkcrc_fail;
	}

	if ((unsigned int)sbuf.st_size < (sizeof(image_header_t) + (2 * 1024 * 1024)) || 
	    (unsigned int)sbuf.st_size > get_mtd_size(FW_MTD_NAME)) {
		fprintf(stderr, "Firmware image size is invalid!\n");
		ret=-1;
		goto checkcrc_fail;
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "Can't map %s: %s\n", fw_image, strerror(errno));
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
		goto checkcrc_fail;
	}

	datalen = SWAP_LONG(hdr->ih_size);
	if (datalen > ((unsigned int)sbuf.st_size - sizeof(image_header_t)))
	{
		ret=-1;
		goto checkcrc_fail;
	}

	checksum = crc32_sp(0, (const char *)(ptr + sizeof(image_header_t)), datalen);

	if (checksum!=SWAP_LONG(hdr->ih_dcrc))
	{
		ret=-1;
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
		fprintf (stderr, "Read error on %s: %s\n",
			fw_image, strerror(errno));
		ret=-1;
	}
checkcrc_end:
	return ret;
}

static int chk_image_err = 1;

static void
do_upgrade_post(char *url, FILE *stream, int len, char *boundary)
{
	#define MAX_VERSION_LEN 64
	char upload_fifo[] = "/tmp/linux.trx";
	FILE *fifo = NULL;
	char buf[4096];
	int count, ret = EINVAL, ch;
	int cnt;
	long filelen, *filelenptr, tmp;
	char cmpHeader = 0;
	
	// delete some files (need free space in /tmp)
	unlink("/tmp/usb.log");
	unlink("/tmp/syscmd.log");
	system("rm -rf /tmp/xupnpd-cache");
	system("rm -rf /tmp/xupnpd-feeds");

	eval("/sbin/stopservice", "99");

	chk_image_err = 1;

	/* Look for our part */
	while (len > 0) 
	{
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
		{
			goto err;
		}

		len -= strlen(buf);

		if (!strncasecmp(buf, "Content-Disposition:", 20) && strstr(buf, "name=\"file\""))
			break;
	}

	/* Skip boundary and headers */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream))
		{
			goto err;
		}
		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n"))
		{
			break;
		}
	}

	if (!(fifo = fopen(upload_fifo, "w+"))) goto err;

	filelen = len;
	cnt = 0;

	/* Pipe the rest to the FIFO */
	cmpHeader = 0;

	while (len>0 && filelen>0) 
	{
		if (waitfor (fileno(stream), 10) <= 0)
		{
			break;
		}

		count = fread(buf, 1, MIN(len, sizeof(buf)), stream);

		if (cnt==0 && count>48)
		{
			if (!(	buf[0] == 0x27 &&	/* Image Magic Number: 0x27051956 */
				buf[1] == 0x05 &&
				buf[2] == 0x19 &&
				buf[3] == 0x56)
			)
			{
				fprintf(stderr, "Header %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);// 520N debug
				len-=count;
				goto err;
			}
			
			if (strncmp(buf+36, BOARD_PID, 7)==0)
				cmpHeader=1;
			else
			{
				fprintf(stderr, "Wrong PID!\n");
				len-=count;
				goto err;
			}

			filelenptr=(long*)(buf+12);
			tmp=*filelenptr;
			filelen=SWAP_LONG(tmp);
			filelen+=64;
			cnt++;
		}
		filelen-=count;
		len-=count;
		fwrite(buf, 1, count, fifo);
	}

	if (!cmpHeader) goto err;

	/* Slurp anything remaining in the request */
	while (len-- > 0)
	{
		ch = fgetc(stream);
		if (filelen>0)
		{
			fwrite(&ch, 1, 1, fifo);
			filelen--;
		}
	}

	fseek(fifo, 0, SEEK_END);
	fclose(fifo);
	fifo = NULL;

	ret = checkcrc(upload_fifo);

 err:
	if (fifo)
		fclose(fifo);

	/* Slurp anything remaining in the request */
	while (len-- > 0)
		ch = fgetc(stream);
	
	if ((ret == 0) && (cmpHeader))
		chk_image_err = 0;
}

static void
do_upgrade_cgi(char *url, FILE *stream)
{
	int i, success = 0;
	char *firmware_image = "/tmp/linux.trx";
	
	if (chk_image_err == 0)
	{
		system("killall -q watchdog");
		system("killall -q ip-up");
		system("killall -q ip-down");
		system("killall -q l2tpd");
		system("killall -q xl2tpd");
		system("killall -q pppd");
		system("killall -q wpa_cli");
		system("killall -q wpa_supplicant");
		
		/* save storage (if changed) */
		system("/sbin/mtd_storage.sh save");
		
		/* wait pppd finished */
		for (i=0; i<5; i++) {
			if (!pids("pppd"))
				break;
			sleep(1);
		}
		
		websApply(stream, "Updating.asp");
		system("cp -f /bin/mtd_write /tmp");
		if (eval("/tmp/mtd_write", "-r", "write", firmware_image, FW_MTD_NAME) == 0) {
			success = 1;
		}
	}
	
	if (!success) {
		websApply(stream, "UpdateError.asp");
		unlink(firmware_image);
	}
}

static void
do_upload_post(char *url, FILE *stream, int len, char *boundary)
{
	#define MAX_VERSION_LEN 64
	char upload_fifo[] = "/tmp/settings_u.prf";
	FILE *fifo = NULL;
	char buf[1024];
	int count, ret = EINVAL, ch;
	int cnt;
	long filelen, *filelenptr;
	char cmpHeader;

	cprintf("Start upload\n");

	/* Look for our part */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream)) {
			goto err;
		}

		len -= strlen(buf);

		if (!strncasecmp(buf, "Content-Disposition:", 20)
				&& strstr(buf, "name=\"file\""))
			break;
	}

	/* Skip boundary and headers */
	while (len > 0) {
		if (!fgets(buf, MIN(len + 1, sizeof(buf)), stream)) {
			goto err;
		}

		len -= strlen(buf);
		if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n")) {
			break;
		}
	}

	if (!(fifo = fopen(upload_fifo, "a+")))
		goto err;

	filelen = len;
	cnt = 0;

	/* Pipe the rest to the FIFO */
	cprintf("Upgrading %d\n", len);
	cmpHeader = 0;

	while (len > 0 && filelen > 0) {
		if (waitfor (fileno(stream), 10) <= 0) {
			break;
		}

		count = fread(buf, 1, MIN(len, sizeof(buf)), stream);

		if (cnt == 0 && count > 8) {
			if (!strncmp(buf, PROFILE_HEADER, 4))
			{
				filelenptr = (long*)(buf + 4);
				filelen = *filelenptr;
			}
			else if (!strncmp(buf, PROFILE_HEADER_NEW, 4))
			{
				filelenptr = (long*)(buf + 4);
				filelen = *filelenptr;
				filelen = filelen & 0xffffff;
			}
			else
			{
				len -= count;
				goto err;
			}

			cmpHeader = 1;
			++cnt;
		}

		filelen -= count;
		len -= count;

		fwrite(buf, 1, count, fifo);
	}

	if (!cmpHeader)
		goto err;

	/* Slurp anything remaining in the request */
	while (len-- > 0) {
		ch = fgetc(stream);

		if (filelen > 0) {
			fwrite(&ch, 1, 1, fifo);
			--filelen;
		}
	}

	ret = 0;

	fseek(fifo, 0, SEEK_END);
	fclose(fifo);
	fifo = NULL;

err:
	if (fifo)
		fclose(fifo);

	/* Slurp anything remaining in the request */
	while (len-- > 0)
		ch = fgetc(stream);

	fcntl(fileno(stream), F_SETOWN, -ret);
}

static void
do_upload_cgi(char *url, FILE *stream)
{
	int ret;
	
	ret = fcntl(fileno(stream), F_GETOWN, 0);
	
	/* Reboot if successful */
	if (ret == 0)
	{
		websApply(stream, "Uploading.asp");
		system("killall -q watchdog");
		sleep(1);
		sys_upload("/tmp/settings_u.prf");
		nvram_commit_safe();
		sys_reboot();
	}
	else
	{
		websApply(stream, "UploadError.asp");
		//unlink("/tmp/settings_u.prf");
	}
}

// Viz 2010.08
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
                                        if ((argv[argc] = (char *)get_cgi(s)) == NULL) break;
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
	char *nvram_file="/tmp/settings_d.prf";

	unlink(nvram_file);
	sys_download(nvram_file);
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
#endif

static char no_cache[] =
"Cache-Control: no-cache\r\n"
"Pragma: no-cache\r\n"
"Expires: 0"
;

static void 
do_log_cgi(char *path, FILE *stream)
{
	dump_file(stream, "/tmp/syslog.log-1");
	dump_file(stream, "/tmp/syslog.log");
	fputs("\r\n", stream); /* terminator */
	fputs("\r\n", stream); /* terminator */
}

//2008.08 magic{
struct mime_handler mime_handlers[] = {
	{ "Nologin.asp", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "error_page.htm*", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "gotoHomePage.htm", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "ure_success.htm", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "ureip.asp", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "remote.asp", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	//{ "jquery.js", "text/javascript", no_cache_IE7, NULL, do_file, NULL }, // 2010.09 James.
	{ "jquery.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**bootstrap.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**engage.itoggle.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**engage.itoggle.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**engage.itoggle.min.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**highstock.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	
	{ "**AiDisk_folder_tree.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**formcontrol.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "ajax.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "alttxt.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "cdma2000_list.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "client_function.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "detectWAN.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "disk_functions.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "md5.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tablesorter.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "td-scdma_list.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmcal.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmhist.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "tmmenu.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "wcdma_list.js", "text/javascript", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23

	{ "httpd_check.htm", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, NULL },
	{ "**.htm*", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, do_auth },
	{ "**.asp*", "text/html", no_cache_IE9, do_html_post_and_get, do_ej, do_auth },
	
	//{ "**.css", "text/css", NULL, NULL, do_file, NULL },
	//{ "**.png", "image/png", NULL, NULL, do_file, NULL },
	//{ "**.gif", "image/gif", NULL, NULL, do_file, NULL },
	//{ "**.jpg", "image/jpeg", NULL, NULL, do_file, NULL },
	{ "**.css", "text/css", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.png", "image/png", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.gif", "image/gif", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23
	{ "**.jpg", "image/jpeg", cache_static, NULL, do_file, NULL }, // 2012.06 Eagle23


	// Viz 2010.08
        { "**.svg", "image/svg+xml", NULL, NULL, do_file, NULL },
        { "**.swf", "application/x-shockwave-flash", NULL, NULL, do_file, NULL  },
        { "**.htc", "text/x-component", NULL, NULL, do_file, NULL  },
	// end Viz

#ifdef TRANSLATE_ON_FLY
	/* Only general.js and quick.js are need to translate. (reduce translation time) */
	{ "general.js|quick.js",  "text/javascript", no_cache_IE9, NULL, do_ej, do_auth },
#endif //TRANSLATE_ON_FLY
	
	{ "**.js",  "text/javascript", no_cache_IE9, NULL, do_ej, do_auth },
	{ "**.cab", "text/txt", NULL, NULL, do_file, do_auth },
	{ "**.CFG", "application/force-download", NULL, NULL, do_prf_file, do_auth },
	
	{ "apply.cgi*", "text/html", no_cache_IE9, do_html_post_and_get, do_apply_cgi, do_auth },
	{ "upgrade.cgi*", "text/html", no_cache_IE9, do_upgrade_post, do_upgrade_cgi, do_auth},
	{ "upload.cgi*", "text/html", no_cache_IE9, do_upload_post, do_upload_cgi, do_auth },
	{ "syslog.cgi*", "application/force-download", no_cache_IE9, do_html_post_and_get, do_log_cgi, do_auth },
        // Viz 2010.08 vvvvv  
        { "update.cgi*", "text/javascript", no_cache_IE9, do_html_post_and_get, do_update_cgi, do_auth }, // jerry5 
        // end Viz  ^^^^^^^^ 
#ifdef TRANSLATE_ON_FLY
	{ "change_lang.cgi*", "text/html", no_cache_IE9, do_lang_post, do_lang_cgi, do_auth },
#endif //TRANSLATE_ON_FLY
	{ "webcam.cgi*", "text/html", no_cache_IE9, NULL, do_webcam_cgi, do_auth },
	{ NULL, NULL, NULL, NULL, NULL, NULL }
};
//2008.08 magic}

int ej_get_AiDisk_status(int eid, webs_t wp, int argc, char **argv) {
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	char *follow_info;
	int sh_num;
	char **folder_list = NULL;
	int first_pool, first_folder, result, i;

	websWrite(wp, "function get_cifs_status() {\n");
	//websWrite(wp, "    return %d;\n", nvram_get_int("samba_running"));
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_samba"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_ftp_status() {\n");
	//websWrite(wp, "    return %d;\n", nvram_get_int("ftp_running"));
	websWrite(wp, "    return %d;\n", nvram_get_int("enable_ftp"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_dms_status() {\n");
	websWrite(wp, "    return %d;\n", pids("ushare"));
	websWrite(wp, "}\n\n");

	websWrite(wp, "function get_share_management_status(protocol) {\n");
	websWrite(wp, "    if (protocol == \"cifs\")\n");
	websWrite(wp, "	return %d;\n", nvram_get_int("st_samba_mode"));
	websWrite(wp, "    else if (protocol == \"ftp\")\n");
	websWrite(wp, "    if (protocol == \"ftp\")\n");
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
		free_disk_data(&disks_info);
	}

	return 0;
}

int ej_get_all_accounts(int eid, webs_t wp, int argc, char **argv) {
	int acc_num;
	char **account_list = NULL;
	int result, i, first;

	if ((result = get_account_list(&acc_num, &account_list)) < 0) {
		printf("Failed to get the account list!\n");
		return -1;
	}

	first = 1;
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

int
count_sddev_mountpoint()
{
	FILE *fp;
	char line[256], devname[32], mpname[32], system_type[10], mount_mode[96];
	int dummy1, dummy2, count = 0;

	fp = fopen("/proc/mounts", "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp))
		{
			if (sscanf(line, "%s %s %s %s %d %d", devname, mpname, system_type, mount_mode, &dummy1, &dummy2) != 6)
				continue;
				
			if (strstr(devname, "/dev/sd"))
				count++;
		}
		fclose(fp);
	}

	return count;
}

void start_flash_usbled()
{
	system("killall -SIGUSR1 detect_link");
}

void stop_flash_usbled()
{
	system("killall -SIGUSR2 detect_link");
}

static int ej_safely_remove_disk(int eid, webs_t wp, int argc, char_t **argv) 
{
	int result = -1;
        char *disk_port = websGetVar(wp, "disk", "");

	csprintf("disk_port = %s\n", disk_port);

	start_flash_usbled();

	if (!disk_port)
		;
	else if (atoi(disk_port) == 1)
		result = eval("/sbin/ejusb1");
	else if (atoi(disk_port) == 2)
		result = eval("/sbin/ejusb2");

	if (result != 0) {
		show_error_msg("Action9");

		websWrite(wp, "<script>\n");
		websWrite(wp, "safely_remove_disk_error(\'%s\');\n", error_msg);
		websWrite(wp, "</script>\n");
		clean_error_msg();
		stop_flash_usbled();
		return -1;
	}
	websWrite(wp, "<script>\n");
	websWrite(wp, "safely_remove_disk_success(\'%s\');\n", error_msg);
	websWrite(wp, "</script>\n");

	stop_flash_usbled();
	return 0;
}



int ej_get_permissions_of_account(int eid, webs_t wp, int argc, char **argv) {
	disk_info_t *disks_info, *follow_disk;
	partition_info_t *follow_partition;
	int acc_num = 0, sh_num = 0;
	char **account_list = NULL, **folder_list;
	int samba_right, ftp_right, dms_right;
	int result, i, j;
	int first_pool, first_account, first_folder;

	//printf("[httpd] get permission of account chk\n");	// tmp test

	disks_info = read_disk_data();
	if (disks_info == NULL) {
		websWrite(wp, "function get_account_permissions_in_pool(account, pool) {return [];}\n");
		websWrite(wp, "function get_dms_permissions_in_pool(pool) {return [];}\n");
		return -1;
	}

	//printf("chk a\n");	// tmp test
	result = get_account_list(&acc_num, &account_list);
	//printf("chk b\n");	// tmp test

	if (result < 0) {
		printf("1. Can't get the account list.\n");
		free_2_dimension_list(&acc_num, &account_list);
		free_disk_data(&disks_info);
	}

	websWrite(wp, "function get_account_permissions_in_pool(account, pool) {\n");

	if (acc_num <= 0)
		websWrite(wp, "    return [];\n");

	first_account = 1;
	for (i = 0; i < acc_num; ++i) {
		websWrite(wp, "    ");
		if (first_account == 1)
			first_account = 0;
		else
			websWrite(wp, "else ");

		websWrite(wp, "if (account == \"%s\") {\n", account_list[i]);

		first_pool = 1;
		for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
			for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
				if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
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

						printf("get_permissions_of_account1: Can't get all folders in \"%s\".\n", follow_partition->mount_point);

						free_2_dimension_list(&sh_num, &folder_list);

						continue;
					}
					first_folder = 1;
					for (j = 0; j < sh_num; ++j) {
						samba_right = get_permission(account_list[i],
													 follow_partition->mount_point,
													 folder_list[j],
													 "cifs");
						ftp_right = get_permission(account_list[i],
												   follow_partition->mount_point,
												   folder_list[j],
												   "ftp");
						if (samba_right < 0 || samba_right > 3) {
							printf("Can't get the CIFS permission abount \"%s\"!\n", folder_list[j]);

//							samba_right = DEFAULT_SAMBA_RIGHT;	// J++
							samba_right = 0;	
						}

						if (ftp_right < 0 || ftp_right > 3) {
							printf("Can't get the FTP permission abount \"%s\"!\n", folder_list[j]);

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
				}
			}
		}

		websWrite(wp, "    }\n");
	}

	websWrite(wp, "}\n\n");

	if (sh_num > 0)
		free_2_dimension_list(&sh_num, &folder_list);

	websWrite(wp, "function get_dms_permissions_in_pool(pool) {\n");

	first_pool = 1;
	for (follow_disk = disks_info; follow_disk != NULL; follow_disk = follow_disk->next) {
		for (follow_partition = follow_disk->partitions; follow_partition != NULL; follow_partition = follow_partition->next) {
			if (follow_partition->mount_point != NULL && strlen(follow_partition->mount_point) > 0) {
				websWrite(wp, "    ");
				if (first_pool == 1)
					first_pool = 0;
				else
					websWrite(wp, "else ");
				websWrite(wp, "if (pool == \"%s\") {\n", rindex(follow_partition->mount_point, '/')+1);

				websWrite(wp, "	return [");

				result = get_all_folder_in_mount_path(follow_partition->mount_point, &sh_num, &folder_list);
				if (result != 0) {
					websWrite(wp, "];\n");
					websWrite(wp, "    }\n");

					printf("get_permissions_of_account2: Can't get all folders in \"%s\".\n", follow_partition->mount_point);

					free_2_dimension_list(&sh_num, &folder_list);

					continue;
				}

				//printf("chk p3\n");	// tmp test
				first_folder = 1;
				for (j = 0; j < sh_num; ++j) {
					dms_right = get_permission("MediaServer",
											   follow_partition->mount_point,
											   folder_list[j],
											   "dms");
					if (dms_right < 0 || dms_right > 1) {
						printf("Can't get the DMS permission abount \"%s\"!\n", folder_list[j]);

						dms_right = DEFAULT_DMS_RIGHT;
					}

					if (first_folder == 1) {
						first_folder = 0;
						websWrite(wp, "[\"%s\", %d]", folder_list[j], dms_right);
					}
					else
						websWrite(wp, "		[\"%s\", %d]", folder_list[j], dms_right);

					if (j != sh_num-1)
						websWrite(wp, ",\n");
				}
				//printf("chk p4\n");	// tmp test

				websWrite(wp, "];\n");
				websWrite(wp, "    }\n");
			}
		}
	}

	websWrite(wp, "}\n\n");

	if (acc_num > 0)
		free_2_dimension_list(&acc_num, &account_list);

	if (sh_num > 0)
		free_2_dimension_list(&sh_num, &folder_list);

	if (disks_info != NULL)
		free_disk_data(&disks_info);

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

				free_disk_data(&disks_info);

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
	free_disk_data(&disks_info);
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

	free_disk_data(&disks_info);

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

	free_disk_data(&disks_info);
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

	printf("[httpd] set share mode: prot = %s\n", protocol);	// tmp test
	if (strlen(dummyShareway) > 0)
		nvram_set("dummyShareway", dummyShareway);
	else
		nvram_set("dummyShareway", "0");

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

			nvram_set("st_samba_mode", "1");	// for test
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 1)
				goto SET_SHARE_MODE_SUCCESS;

			nvram_set("st_ftp_mode", "1");
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

			nvram_set("st_samba_mode", "4");
		}
		else if (!strcmp(protocol, "ftp")) {
			if (ftp_mode == 2)
				goto SET_SHARE_MODE_SUCCESS;

			nvram_set("st_ftp_mode", "2");
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
		nvram_set("st_ftp_mode", "3");
	}
	else if (!strcmp(mode, "account_anonym") && !strcmp(protocol, "ftp")) {
		if (ftp_mode == 4)
			goto SET_SHARE_MODE_SUCCESS;
		nvram_set("st_ftp_mode", "4");
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
			nvram_set("enable_samba", "1");
			nvram_commit_safe();
			result = system("/sbin/run_samba");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set("enable_samba", "0");
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
			nvram_set("enable_ftp", "1");
			nvram_commit_safe();
			result = system("run_ftp");
		}
		else if (!strcmp(flag, "off")) {
			nvram_set("enable_ftp", "0");
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
	nvram_set("acc_num", "0");
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

	free_disk_data(&disks_info);

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

// 2010.09 James. {
int start_mac_clone(int eid, webs_t wp, int argc, char **argv) {
	char *cmd[] = {"start_mac_clone", NULL};
	int pid;

	dbg("start mac clone...\n");
	_eval(cmd, NULL, 0, &pid);
	return 0;
}

#define MAX_SUBNET (0xC0A8FF00)
#define MIN_LAN_IP (0xC0A80001)

int setting_lan(int eid, webs_t wp, int argc, char **argv) {
	char lan_ipaddr_t[16];
	char lan_netmask_t[16];
	unsigned int lan_ip_num;
	unsigned int lan_mask_num;
	unsigned int lan_subnet;
	char wan_ipaddr_t[16];
	char wan_netmask_t[16];
	unsigned int wan_ip_num;
	unsigned int wan_mask_num;
	unsigned int wan_subnet;
	struct in_addr addr;
	unsigned int new_lan_ip_num;
	unsigned int new_dhcp_start_num;
	unsigned int new_dhcp_end_num;
	char new_lan_ip_str[16];
	char new_dhcp_start_str[16];
	char new_dhcp_end_str[16];

	memset(lan_ipaddr_t, 0, 16);
	strcpy(lan_ipaddr_t, nvram_safe_get("lan_ipaddr_t"));
	memset(&addr, 0, sizeof(addr));
	lan_ip_num = ntohl(inet_aton(lan_ipaddr_t, &addr));
	lan_ip_num = ntohl(addr.s_addr);
	memset(lan_netmask_t, 0, 16);
	strcpy(lan_netmask_t, nvram_safe_get("lan_netmask_t"));
	memset(&addr, 0, sizeof(addr));
	lan_mask_num = ntohl(inet_aton(lan_netmask_t, &addr));
	lan_mask_num = ntohl(addr.s_addr);
	lan_subnet = lan_ip_num&lan_mask_num;
	memset(wan_ipaddr_t, 0, 16);
	strcpy(wan_ipaddr_t, nvram_safe_get("wan_ipaddr_t"));
	memset(&addr, 0, sizeof(addr));
	wan_ip_num = ntohl(inet_aton(wan_ipaddr_t, &addr));
	wan_ip_num = ntohl(addr.s_addr);
	memset(wan_netmask_t, 0, 16);
	strcpy(wan_netmask_t, nvram_safe_get("wan_netmask_t"));
	memset(&addr, 0, sizeof(addr));
	wan_mask_num = ntohl(inet_aton(wan_netmask_t, &addr));
	wan_mask_num = ntohl(addr.s_addr);
	wan_subnet = wan_ip_num&wan_mask_num;
	
	if (strcmp(nvram_safe_get("wan_ready"), "1")
			|| lan_subnet != wan_subnet
			) {
		websWrite(wp, "0");
		return 0;
	}
	
	if (lan_subnet >= MAX_SUBNET)
		new_lan_ip_num = MIN_LAN_IP;
	else
		new_lan_ip_num = lan_ip_num+(~lan_mask_num)+1;
	
	new_dhcp_start_num = new_lan_ip_num+1;
	new_dhcp_end_num = new_lan_ip_num+(~inet_network(lan_netmask_t))-2;
	memset(&addr, 0, sizeof(addr));
	addr.s_addr = htonl(new_lan_ip_num);
	memset(new_lan_ip_str, 0, 16);
	strcpy(new_lan_ip_str, inet_ntoa(addr));
	memset(&addr, 0, sizeof(addr));
	addr.s_addr = htonl(new_dhcp_start_num);
	memset(new_dhcp_start_str, 0, 16);
	strcpy(new_dhcp_start_str, inet_ntoa(addr));
	memset(&addr, 0, sizeof(addr));
	addr.s_addr = htonl(new_dhcp_end_num);
	memset(new_dhcp_end_str, 0, 16);
	strcpy(new_dhcp_end_str, inet_ntoa(addr));
	nvram_set("lan_ipaddr", new_lan_ip_str);
	nvram_set("dhcp_start", new_dhcp_start_str);
	nvram_set("dhcp_end", new_dhcp_end_str);
	
	websWrite(wp, "1");
	
	nvram_commit_safe();
	
	sys_reboot();
	return 0;
}
// 2010.09 James. }

// qos svg support 2010.08 Viz vvvvvvvvvvvv
static int
asp_ctcount(webs_t wp, int argc, char_t **argv)
{
	static const char *states[10] = {
		"NONE", "ESTABLISHED", "SYN_SENT", "SYN_RECV", "FIN_WAIT",
		"TIME_WAIT", "CLOSE", "CLOSE_WAIT", "LAST_ACK", "LISTEN" };
	int count[13];	// tcp(10) + udp(2) + total(1) = 13 / max classes = 10
	FILE *f;
	char s[512];
	char *p;
	int i;
	int n;
	int mode;
	unsigned long rip;
	unsigned long lan;
	unsigned long mask;
	int ret = 0;

	if (argc != 1) return 0;
	mode = atoi(argv[0]);

	memset(count, 0, sizeof(count));
	

	  if ((f = fopen("/proc/net/ip_conntrack", "r")) != NULL) {   
		// ctvbuf(f);	// if possible, read in one go

		if (nvram_match("t_hidelr", "1")) {
			mask = inet_addr(nvram_safe_get("lan_netmask"));
			rip = inet_addr(nvram_safe_get("lan_ipaddr"));
			lan = rip & mask;
		}
		else {
			rip = lan = mask = 0;
		}

		while (fgets(s, sizeof(s), f)) {
			if (rip != 0) {
				// src=x.x.x.x dst=x.x.x.x	// DIR_ORIGINAL
				if ((p = strstr(s + 14, "src=")) == NULL) continue;
				if ((inet_addr(p + 4) & mask) == lan) {
					if ((p = strstr(p + 13, "dst=")) == NULL) continue;
					if (inet_addr(p + 4) == rip) continue;
				}
			}

			if (mode == 0) {
				// count connections per state
				if (strncmp(s, "tcp", 3) == 0) {
					for (i = 9; i >= 0; --i) {
						if (strstr(s, states[i]) != NULL) {
							count[i]++;
							break;
						}
					}
				}
				else if (strncmp(s, "udp", 3) == 0) {
					if (strstr(s, "[UNREPLIED]") != NULL) {
						count[10]++;
					}
					else if (strstr(s, "[ASSURED]") != NULL) {
						count[11]++;
					}
				}
				count[12]++;
			}
			else {
				// count connections per mark
				if ((p = strstr(s, " mark=")) != NULL) {
					n = atoi(p + 6) & 0xFF;
					if (n <= 10) count[n]++;
				}
			}
		}

		fclose(f);
	}

	if (mode == 0) {
		p = s;
		for (i = 0; i < 12; ++i) {
			p += sprintf(p, ",%d", count[i]);
		}
		ret += websWrite(wp, "\nconntrack = [%d%s];\n", count[12], s);
	}
	else {
		p = s;
		for (i = 1; i < 11; ++i) {
			p += sprintf(p, ",%d", count[i]);
		}
		ret += websWrite(wp, "\nnfmarks = [%d%s];\n", count[0], s);
	}
	
	return 0;
}

static int
ej_qos_packet(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *f;
	char s[256];
	unsigned long rates[10];
	unsigned long u;
	char *e;
	int n;
	char comma;
	char *a[1];
	int ret = 0;

	a[0] = "1";
  asp_ctcount(wp, 1, a);

	memset(rates, 0, sizeof(rates));
	sprintf(s, "tc -s class ls dev %s", nvram_safe_get("wan0_ifname"));
	if ((f = popen(s, "r")) != NULL) {
		n = 1;
		while (fgets(s, sizeof(s), f)) {
			if (strncmp(s, "class htb 1:", 12) == 0) {
				n = atoi(s + 12);
			}
			else if (strncmp(s, " rate ", 6) == 0) {
				if ((n % 10) == 0) {
					n /= 10;
					if ((n >= 1) && (n <= 10)) {
						u = strtoul(s + 6, &e, 10);
						if (*e == 'K') u *= 1000;
							else if (*e == 'M') u *= 1000 * 1000;
						rates[n - 1] = u;
						n = 1;
					}
				}
			}
		}
		pclose(f);
	}

	comma = ' ';
	ret += websWrite(wp, "\nqrates = [0,");
	for (n = 0; n < 10; ++n) {
		ret += websWrite(wp, "%c%lu", comma, rates[n]);
		comma = ',';
	}
	ret += websWrite(wp, "];");

	return ret;
}

static int
ej_ctdump(int eid, webs_t wp, int argc, char **argv)
{
	FILE *f;
	char s[512];
	char *p, *q;
	int mark;
	int findmark;
	unsigned int proto;
	unsigned int time;
	char src[16];
	char dst[16];
	char sport[16];
	char dport[16];
	unsigned long rip;
	unsigned long lan;
	unsigned long mask;
	char comma;
	int ret = 0;

	if (argc != 1) return 0;

	findmark = atoi(argv[0]);

	mask = inet_addr(nvram_safe_get("lan_netmask"));
	rip = inet_addr(nvram_safe_get("lan_ipaddr"));
	lan = rip & mask;
	if (nvram_match("t_hidelr", "0")) rip = 0;	// hide lan -> router?

	ret += websWrite(wp, "\nctdump = [");
	comma = ' ';
	if ((f = fopen("/proc/net/ip_conntrack", "r")) != NULL) {
		//ctvbuf(f);
		while (fgets(s, sizeof(s), f)) {
			if ((p = strstr(s, " mark=")) == NULL) continue;
			if ((mark = (atoi(p + 6) & 0xFF)) > 10) mark = 0;
			if ((findmark != -1) && (mark != findmark)) continue;

			if (sscanf(s, "%*s %u %u", &proto, &time) != 2) continue;

			if ((p = strstr(s + 14, "src=")) == NULL) continue;		// DIR_ORIGINAL
			if ((inet_addr(p + 4) & mask) != lan) {
				// make sure we're seeing int---ext if possible
				if ((p = strstr(p + 41, "src=")) == NULL) continue;	// DIR_REPLY
			}
			else if (rip != 0) {
				if ((q = strstr(p + 13, "dst=")) == NULL) continue;
//				cprintf("%lx=%lx\n", inet_addr(q + 4), rip);
				if (inet_addr(q + 4) == rip) continue;
			}

			if ((proto == 6) || (proto == 17)) {
				if (sscanf(p + 4, "%s dst=%s sport=%s dport=%s", src, dst, sport, dport) != 4) continue;
			}
			else {
				if (sscanf(p + 4, "%s dst=%s", src, dst) != 2) continue;
				sport[0] = 0;
				dport[0] = 0;
			}
			ret += websWrite(wp, "%c[%u,%u,'%s','%s','%s','%s',%d]", comma, proto, time, src, dst, sport, dport, mark);
			comma = ',';
		}
	}
	ret += websWrite(wp, "];\n");

	return ret;
}

// traffic monitor
static int 
ej_netdev(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE * fp;
	char buf[256];
	uint64_t rx, tx;
	char *p;
	char *ifname;
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
			
			if ( (strcmp(ifname, "ra0") != 0) &&
			     (strcmp(ifname, "rai0") != 0) &&
#ifdef USE_SINGLE_MAC
			     (strcmp(ifname, "eth2.2") != 0) &&
			     (strcmp(ifname, "eth2.1") != 0) )
#else
			     (strcmp(ifname, "eth3") != 0) &&
			     (strcmp(ifname, "eth2") != 0) )
#endif
				continue;
			
			if (sscanf(p + 1, "%llu%*u%*u%*u%*u%*u%*u%*u%llu", &rx, &tx) != 2) continue;
			ret += websWrite(wp, "%c'%s':{rx:0x%llx,tx:0x%llx}", comma, ifname, rx, tx);
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
	char *name;
	int sig;

	if (strcmp(argv[0], "speed") == 0) {
		sig = SIGUSR1;
		name = "/var/spool/rstats-speed.js";
	}
	else {
		sig = SIGUSR2;
		name = "/var/spool/rstats-history.js";
	}

	if (pids("rstats")) {
		unlink(name);
		killall("rstats", sig);
		f_wait_exists(name, 5);
		do_f(name, wp);
	}
	else {
		if (f_exists(name)) {
			do_f(name, wp);
		}
		else {
			if (sig == SIGUSR1)
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
		v = nvram_get(k);
		if (!v) {
			v = "";
		}
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
	{ "nvram_get_f", ej_nvram_get_f},
	{ "nvram_get_list_x", ej_nvram_get_list_x},
	{ "nvram_get_buf_x", ej_nvram_get_buf_x},
	{ "nvram_get_table_x", ej_nvram_get_table_x},
	{ "nvram_match_x", ej_nvram_match_x},
	{ "nvram_double_match_x", ej_nvram_double_match_x},
	{ "nvram_match_both_x", ej_nvram_match_both_x},
	{ "nvram_match_list_x", ej_nvram_match_list_x},
	{ "select_channel", ej_select_channel},
	{ "urlcache", ej_urlcache},     
	{ "uptime", ej_uptime},
	{ "sysuptime", ej_sysuptime},
	{ "nvram_dump", ej_dump},
	{ "load_script", ej_load},
	{ "select_list", ej_select_list},

//tomato qosvvvvvvvvvvv 2010.08 Viz
        { "qrate", ej_qos_packet},
        { "ctdump", ej_ctdump},
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
	{ "detect_wan_connection", detect_wan_connection},
	{ "detect_dhcp_pppoe", detect_dhcp_pppoe},
	{ "get_wan_status_log", get_wan_status_log},
	{ "wanlink", wanlink_hook},
	{ "lanlink", lanlink_hook},
	{ "wan_action", wan_action_hook},
	{ "wol_action", wol_action_hook},
	{ "nf_values", nf_values_hook},
	{ "get_parameter", ej_get_parameter},
	{ "login_state_hook", login_state_hook},
	{ "get_nvram_list", ej_get_nvram_list},
	{ "dhcp_leases", ej_dhcp_leases},
	{ "get_arp_table", ej_get_arp_table},
	{ "get_static_client", ej_get_static_client},
	{ "get_vpns_client", ej_get_vpns_client},
	{ "get_changed_status", ej_get_changed_status},
	{ "wl_auth_list", ej_wl_auth_list},
	{ "wl_scan_5g", ej_wl_scan_5g},
	{ "wl_scan_2g", ej_wl_scan_2g},
	{ "wl_bssid_5g", ej_wl_bssid_5g},
	{ "wl_bssid_2g", ej_wl_bssid_2g},
	{ "ej_system_status", ej_system_status_hook},
	{ "shown_language_option", ej_shown_language_option},
	{ "disk_pool_mapping_info", ej_disk_pool_mapping_info},
	{ "available_disk_names_and_sizes", ej_available_disk_names_and_sizes},
	{ "get_printer_info", ej_get_printer_info},
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
	{ "start_mac_clone", start_mac_clone},
	{ "setting_lan", setting_lan},
	{ "usb_apps_check", usb_apps_check_hook},
	{ "board_caps_hook", board_caps_hook},
	{ "kernel_caps_hook", kernel_caps_hook},
	{ NULL, NULL }
};



