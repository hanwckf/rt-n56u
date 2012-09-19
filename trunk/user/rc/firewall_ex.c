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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <nvram/bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>

#if defined (USE_KERNEL3X)
#define DAYS_MATCH	" --kerneltz --weekdays "
#else
#define DAYS_MATCH	" --days "
#endif

#define foreach_x(x)	for (i=0; i<nvram_get_int(x); i++)

#define BATTLENET_PORT 6112

char *g_buf;
char g_buf_pool[1024];

int
valid_autofw_port(const netconf_app_t *app)
{
	/* Check outbound protocol */
	if (app->match.ipproto != IPPROTO_TCP && app->match.ipproto != IPPROTO_UDP)
		return 0;

	/* Check outbound port range */
	if (ntohs(app->match.dst.ports[0]) > ntohs(app->match.dst.ports[1]))
		return 0;

	/* Check related protocol */
	if (app->proto != IPPROTO_TCP && app->proto != IPPROTO_UDP)
		return 0;

	/* Check related destination port range */
	if (ntohs(app->dport[0]) > ntohs(app->dport[1]))
		return 0;

	/* Check mapped destination port range */
	if (ntohs(app->to[0]) > ntohs(app->to[1]))
		return 0;

	/* Check port range size */
	if ((ntohs(app->dport[1]) - ntohs(app->dport[0])) !=
	    (ntohs(app->to[1]) - ntohs(app->to[0])))
		return 0;
	
	return 1;
}

void g_buf_init()
{
	g_buf = g_buf_pool;
	g_buf_pool[0] = 0;
}

char *g_buf_alloc(char *g_buf_now)
{
	g_buf += strlen(g_buf_now)+1;

	return (g_buf_now);
}

char *proto_conv(char *proto_name, int idx)
{
	char *proto;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);

	if (!strncasecmp(proto, "Both", 3)) strcpy(g_buf, "both");
	else if (!strncasecmp(proto, "TCP", 3)) strcpy(g_buf, "tcp");
	else if (!strncasecmp(proto, "UDP", 3)) strcpy(g_buf, "udp");
	else if (!strncasecmp(proto, "OTHER", 5)) strcpy(g_buf, "other");
	else strcpy(g_buf,"tcp");

	return (g_buf_alloc(g_buf));
}

char *protoflag_conv(char *proto_name, int idx, int isFlag)
{
	char *proto;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", proto_name, idx);
	proto=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");

	if (!isFlag)
	{
		if (strncasecmp(proto, "UDP", 3)==0) strcpy(g_buf, "udp");
		else strcpy(g_buf, "tcp");
	}
	else
	{
		if (strlen(proto)>3)
		{
			strcpy(g_buf, proto+4);
		}
		else strcpy(g_buf,"");
	}

	return (g_buf_alloc(g_buf));
}

char *portrange_ex2_conv(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");

	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	//else if (strptr=strchr(port, ':'))
	else if ((strptr=strchr(port, ':')) != NULL) //2008.11 magic oleg patch
	{
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d-%d", atoi(itemname_arr), atoi(strptr+1));
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d-%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		//sprintf(g_buf, "");
		 g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}

	return (g_buf_alloc(g_buf));
}

char *portrange_ex2_conv_new(char *port_name, int idx, int *start, int *end)
{
	char *port, *strptr;
	char itemname_arr[32];
	
	sprintf(itemname_arr,"%s%d", port_name, idx);
	port=nvram_safe_get(itemname_arr);

	strcpy(g_buf, "");

	if (!strncmp(port, ">", 1)) 
	{
		sprintf(g_buf, "%d-65535", atoi(port+1) + 1);
		*start=atoi(port+1);
		*end=65535;
	}
	else if (!strncmp(port, "=", 1)) 
	{
		sprintf(g_buf, "%d-%d", atoi(port+1), atoi(port+1));
		*start=*end=atoi(port+1);
	}
	else if (!strncmp(port, "<", 1)) 
	{
		sprintf(g_buf, "1-%d", atoi(port+1) - 1);
		*start=1;
		*end=atoi(port+1);
	}
	else if ((strptr=strchr(port, ':')) != NULL)
	{
		strcpy(itemname_arr, port);
		strptr = strchr(itemname_arr, ':');
		sprintf(g_buf, "%d:%d", atoi(itemname_arr), atoi(strptr+1));
		*start=atoi(itemname_arr);
		*end=atoi(strptr+1);
	}
	else if (*port)
	{
		sprintf(g_buf, "%d:%d", atoi(port), atoi(port));
		*start=atoi(port);
		*end=atoi(port);
	}
	else
	{
		g_buf[0] = 0;	// oleg patch
		*start=0;
		*end=0;
	}

	return (g_buf_alloc(g_buf));
}

char *portrange_conv(char *port_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", port_name, idx);
	strcpy(g_buf, nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

char *ip_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

char *general_conv(char *ip_name, int idx)
{
	char itemname_arr[32];

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	sprintf(g_buf, "%s", nvram_safe_get(itemname_arr));

	return (g_buf_alloc(g_buf));
}

char *filter_conv(char *proto, char *flag, char *srcip, char *srcport, char *dstip, char *dstport)
{
	char newstr[64];

	strcpy(g_buf, "");

	if (strcmp(proto, "")!=0)
	{
		sprintf(newstr, " -p %s", proto);
		strcat(g_buf, newstr);
	}

	if (strcmp(flag, "")!=0)
	{
		sprintf(newstr, " --tcp-flags %s %s", flag, flag);
		strcat(g_buf, newstr);
	}
	
	if (strcmp(srcip, "")!=0)
	{
		if (strchr(srcip , '-'))
			sprintf(newstr, " --src-range %s", srcip);
		else	
			sprintf(newstr, " -s %s", srcip);
		strcat(g_buf, newstr);
	}

	if (strcmp(srcport, "")!=0)
	{
		sprintf(newstr, " --sport %s", srcport);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstip, "")!=0)
	{
		if (strchr(dstip, '-'))
			sprintf(newstr, " --dst-range %s", dstip);
		else	
			sprintf(newstr, " -d %s", dstip);
		strcat(g_buf, newstr);
	}

	if (strcmp(dstport, "")!=0)
	{
		sprintf(newstr, " --dport %s", dstport);
		strcat(g_buf, newstr);
	}

	return (g_buf_alloc(g_buf));
}

void timematch_conv(char *mstr, char *nv_date, char *nv_time)
{
	char *datestr[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char *time, *date;
	int i, comma;

	date = nvram_get(nv_date);
	time = nvram_get(nv_time);

	if (!date) date = "1111111";
	if (!time) time = "00002359";

	mstr[0] = 0;

	if (strlen(date) != 7 || !strcmp(date, "0000000") || strlen(time) != 8)
		return;

	if (strncmp(date, "1111111", 7) == 0 && strncmp(time, "00002359", 8) == 0) 
		return;

	sprintf(mstr, " -m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59%s",
		time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7], DAYS_MATCH);

	comma = 0;
	for (i=0; i<7; i++)
	{
		if (date[i] == '1')
		{
			if (comma)
				strcat(mstr, ",");
			else
				comma = 1;
			
			strcat(mstr, datestr[i]);
		}
	}
}

char *iprange_ex_conv(char *ip_name, int idx)
{
	char *ip;
	char itemname_arr[32];
	char startip[16], endip[16];
	int i, j, k;
	int mask;

	sprintf(itemname_arr,"%s%d", ip_name, idx);
	ip=nvram_safe_get(itemname_arr);
	strcpy(g_buf, "");

	// scan all ip string
	i=j=k=0;
	mask=32;

	while (*(ip+i))
	{
		if (*(ip+i)=='*')
		{
			startip[j++] = '0';
			endip[k++] = '0';
			// 255 is for broadcast
			mask-=8;
		}
		else
		{
			startip[j++] = *(ip+i);
			endip[k++] = *(ip+i);
		}
		++i;
	}

	startip[j++] = 0;
	endip[k++] = 0;

	if (mask==32)
		sprintf(g_buf, "%s", startip);
	else if (mask==0)
		strcpy(g_buf, "");
	else sprintf(g_buf, "%s/%d", startip, mask);

	return (g_buf_alloc(g_buf));
}


int is_valid_filter_date(char *nv_date)
{
	char *date = nvram_get(nv_date);
	if (!date) date = "1111111";

	if (strlen(date) != 7 || !strcmp(date, "0000000"))
		return 0;

	return 1;
}

int is_valid_filter_time(char *nv_time, char *nv_time1, char *nv_enable, char *nv_enable1)
{
	char starttime1[8], endtime1[8];
	char starttime2[8], endtime2[8];
	int  enable1 = nvram_get_int(nv_enable);
	int  enable2 = nvram_get_int(nv_enable1);
	char *time1 = nvram_get(nv_time);
	char *time2 = nvram_get(nv_time1);
	if (!time1) time1 = "00002359";
	if (!time2) time2 = "00002359";

	memset(starttime1, 0, sizeof(starttime1));
	memset(starttime2, 0, sizeof(starttime2));
	memset(endtime1, 0, sizeof(endtime1));
	memset(endtime2, 0, sizeof(endtime2));

	if (enable1 != 1 && enable2 != 1)
		return 0;

	if (enable1 == 1)
	{
		if (strlen(time1) != 8)
			goto err;
		
		strncpy(starttime1, time1, 4);
		strncpy(endtime1, time1 + 4, 4);
		
		if (atoi(starttime1) >= atoi(endtime1))
			goto err;
	}

	if (enable2 == 1)
	{
		if (strlen(time2) != 8)
			goto err;
		
		strncpy(starttime2, time2, 4);
		strncpy(endtime2, time2 + 4, 4);
		
		if (atoi(starttime2) >= atoi(endtime2))
			goto err;
	}

	if (enable1 == 1 && enable2 == 1)
	{
		if ((atoi(starttime1) > atoi(starttime2)) && 
			((atoi(starttime2) > atoi(endtime1)) || (atoi(endtime2) > atoi(endtime1))))
			goto err;
		
		if ((atoi(starttime2) > atoi(starttime1)) && 
			((atoi(starttime1) > atoi(endtime2)) || (atoi(endtime1) > atoi(endtime2))))
			goto err;
	}

	return 1;

err:
	return 0;
}

void 
ip2class(char *lan_ip, char *netmask, char *buf)
{
	unsigned int val, ip;
	struct in_addr in;
	int i=0;

	// only handle class A,B,C
	val = (unsigned int)inet_addr(netmask);
	ip = (unsigned int)inet_addr(lan_ip);

	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++) 
		val <<= 1;

	sprintf(buf, "%s/%d", inet_ntoa(in), i);
}

void convert_routes(void)
{
	int i;
	char *ip, *netmask, *gateway, *metric, *interface;
	char wroutes[1024], lroutes[1024], mroutes[2048];	// oleg patch

	wroutes[0] = 0;
	lroutes[0] = 0;	
	mroutes[0] = 0;	// oleg patch

	if (nvram_match("sr_enable_x", "1"))
	{
		foreach_x("sr_num_x")
		{
			g_buf_init();
			
			ip = general_conv("sr_ipaddr_x", i);
			netmask = general_conv("sr_netmask_x", i);
			gateway = general_conv("sr_gateway_x", i);
			metric = general_conv("sr_matric_x", i);
			interface = general_conv("sr_if_x", i);
			
			if (!strcmp(interface, "WAN"))
			{
				sprintf(wroutes, "%s %s:%s:%s:%d", wroutes, ip, netmask, gateway, atoi(metric));
			}
			else if (!strcmp(interface, "MAN"))	// oleg patch
			{
				sprintf(mroutes, "%s %s:%s:%s:%d", mroutes, ip, netmask, gateway, atoi(metric));
			} 
			else if (!strcmp(interface, "LAN"))
			{
				sprintf(lroutes, "%s %s:%s:%s:%d", lroutes, ip, netmask, gateway, atoi(metric));
			}
		}
	}

	nvram_set("lan_route", lroutes);
	nvram_set("wan0_route", wroutes);
	nvram_set("wan_route", mroutes);	// oleg patch
}

#if defined (USE_KERNEL3X)
int include_port_trigger(FILE *fp, char *wan_if)
{
	netconf_app_t apptarget, *app;
	int i, first, use_autofw;
	char *out_proto, *in_proto, *out_port, *in_port;
	int  out_start, out_end, in_start, in_end;
	
	first = 1;
	use_autofw = 0;
	
	foreach_x("autofw_num_x")
	{
		g_buf_init();
		
		out_proto = proto_conv("autofw_outproto_x", i);
		out_port = portrange_ex2_conv_new("autofw_outport_x", i, &out_start, &out_end);
		in_proto = proto_conv("autofw_inproto_x", i);
		in_port = portrange_ex2_conv("autofw_inport_x", i, &in_start, &in_end);
		app = &apptarget;
		memset(app, 0, sizeof(netconf_app_t));

		/* Parse outbound protocol */
		if (!strncasecmp(out_proto, "tcp", 3))
			app->match.ipproto = IPPROTO_TCP;
		else if (!strncasecmp(out_proto, "udp", 3))
			app->match.ipproto = IPPROTO_UDP;
		else continue;

		/* Parse outbound port range */
		app->match.dst.ports[0] = htons(out_start);
		app->match.dst.ports[1] = htons(out_end);

		/* Parse related protocol */
		if (!strncasecmp(in_proto, "tcp", 3))
			app->proto = IPPROTO_TCP;
		else if (!strncasecmp(in_proto, "udp", 3))
			app->proto = IPPROTO_UDP;
		else continue;

		/* Parse related destination port range */
		app->dport[0] = htons(in_start);
		app->dport[1] = htons(in_end);

		/* Parse mapped destination port range */
		app->to[0] = htons(in_start);
		app->to[1] = htons(in_end);

		/* Set LAN source port range (match packets from any source port) */
		app->match.src.ports[1] = htons(0xffff);

		if (valid_autofw_port(app))
		{
			if (first) {
				fprintf(fp, "-A FORWARD -i %s -j TRIGGER --trigger-type in\n", wan_if);
				first = 0;
			}
			
			fprintf(fp, "-A FORWARD -p %s --dport %s -j TRIGGER --trigger-type out --trigger-proto %s --trigger-match %s --trigger-relate %s\n",
				out_proto, out_port, in_proto, out_port, in_port);
			
			use_autofw = 1;
		}
	}
	
	return use_autofw;
}
#else
int include_porttrigger_preroute(FILE *fp, char *lan_if)
{
	netconf_app_t apptarget, *app;
	int i, use_autofw;
	char *out_proto, *in_proto, *out_port, *in_port;
	int  out_start, out_end, in_start, in_end;
	
	use_autofw = 0;
	
	foreach_x("autofw_num_x")
	{
		g_buf_init();
		
		out_proto = proto_conv("autofw_outproto_x", i);
		out_port = portrange_ex2_conv_new("autofw_outport_x", i, &out_start, &out_end);
		in_proto = proto_conv("autofw_inproto_x", i);
		in_port = portrange_ex2_conv("autofw_inport_x", i, &in_start, &in_end);
		app = &apptarget;
		memset(app, 0, sizeof(netconf_app_t));

		/* Parse outbound protocol */
		if (!strncasecmp(out_proto, "tcp", 3))
			app->match.ipproto = IPPROTO_TCP;
		else if (!strncasecmp(out_proto, "udp", 3))
			app->match.ipproto = IPPROTO_UDP;
		else continue;

		/* Parse outbound port range */
		app->match.dst.ports[0] = htons(out_start);
		app->match.dst.ports[1] = htons(out_end);

		/* Parse related protocol */
		if (!strncasecmp(in_proto, "tcp", 3))
			app->proto = IPPROTO_TCP;
		else if (!strncasecmp(in_proto, "udp", 3))
			app->proto = IPPROTO_UDP;
		else continue;

		/* Parse related destination port range */
		app->dport[0] = htons(in_start);
		app->dport[1] = htons(in_end);

		/* Parse mapped destination port range */
		app->to[0] = htons(in_start);
		app->to[1] = htons(in_end);

		/* Set LAN source port range (match packets from any source port) */
		app->match.src.ports[1] = htons(0xffff);

		if (valid_autofw_port(app))
		{
			/* cmd format:
			 * iptables -t nat -A PREROUTING -i br0 -p INCOMING_PROTOCOL --dport TRIGGER_PORT_FROM(-TRIGGER_PORT_TO) -j autofw --related-proto TRIGGER_PROTOCOL --related-dport INCOMING_PORT_FROM(-INCOMING_PORT_TO) --related-to INCOMING_PORT_FROM(-INCOMING_PORT_TO)
			 *
			 * For example, to set up a trigger for BitTorrent, you'd use this:
			 * iptables -t nat -A PREROUTING -i br0 -p tcp --dport 6881 -j autofw --related-proto tcp --related-dport 6881-6999 --related-to 6881-6999
			 */
			fprintf(fp, "-A PREROUTING -i %s -p %s --dport %s -j autofw --related-proto %s --related-dport %s --related-to %s\n",
				lan_if, out_proto, out_port, in_proto, in_port, in_port);
			
			use_autofw = 1;
		}
	}
	
	return use_autofw;
}
#endif

int include_mac_filter(FILE *fp, char *logdrop)
{
	int i, mac_num, mac_filter;
	char mac_timematch[128], mac_buf[64], nv_date[32], nv_time[32];
	char *filter_mac, *dtype, *ftype;
	
	mac_filter = nvram_get_int("macfilter_enable_x");
	if (mac_filter > 0)
	{
		dtype = "maclist";
		
		if (mac_filter == 2)
			ftype = logdrop;
		else
			ftype = "RETURN";
		
		mac_num = 0;
		foreach_x("macfilter_num_x")
		{
			g_buf_init();
			
			filter_mac = mac_conv("macfilter_list_x", i, mac_buf);
			if (*filter_mac)
			{
				mac_num++;
				sprintf(nv_date, "macfilter_date_x%d", i);
				sprintf(nv_time, "macfilter_time_x%d", i);
				timematch_conv(mac_timematch, nv_date, nv_time);
				fprintf(fp, "-A %s -m mac --mac-source %s%s -j %s\n", dtype, filter_mac, mac_timematch, ftype);
			}
		}
		
		if (mac_filter != 2 && mac_num > 0)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
		
		if (mac_num < 1)
			mac_filter = 0;
	}

	return mac_filter;
}


int include_webstr_filter(FILE *fp)
{
	int i, url_enable, url_enable_1, use_webstr;
	char url_timematch[128], url_timematch_1[128], nvname[32], *filterstr, *dtype;
	
	use_webstr = 0;
	
	if (is_valid_filter_time("url_time_x", "url_time_x_1", "url_enable_x", "url_enable_x_1") && 
	    is_valid_filter_date("url_date_x"))
	{
		dtype = "FORWARD";
		
		url_enable = nvram_get_int("url_enable_x");
		url_enable_1 = nvram_get_int("url_enable_x_1");
		
		if (url_enable == 1)
			timematch_conv(url_timematch, "url_date_x", "url_time_x");
		if (url_enable_1 == 1)
			timematch_conv(url_timematch_1, "url_date_x", "url_time_x_1");
		
		foreach_x("url_num_x")
		{
			sprintf(nvname, "url_keyword_x%d", i);
			filterstr = nvram_safe_get(nvname);
			if (strncasecmp(filterstr, "http://", 7) == 0)
				filterstr += 7;
			else if (strncasecmp(filterstr, "https://", 8) == 0)
				filterstr += 8;
			
			if (*filterstr)
			{
				if (url_enable == 1)
				{
					fprintf(fp, "-A %s -p tcp%s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, url_timematch, filterstr);
					use_webstr = 1;
				}
				
				if (url_enable_1 == 1)
				{
					fprintf(fp, "-A %s -p tcp%s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, url_timematch_1, filterstr);
					use_webstr = 1;
				}
			}
		}
	}

	return use_webstr;
}

void include_lw_filter(FILE *fp, char *lan_if, char *wan_if, char *logaccept, char *logdrop)
{
	int i, lw_enable, lw_enable_1;
	char lw_timematch[128], lw_timematch_1[128], icmp_ptr[64];
	char *icmplist, *filterstr, *dtype, *ftype, *jtype;
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport;

	dtype = "FORWARD";

	if (is_valid_filter_time("filter_lw_time_x", "filter_lw_time_x_1", "fw_lw_enable_x", "fw_lw_enable_x_1") && 
	    is_valid_filter_date("filter_lw_date_x"))
	{
		lw_enable = nvram_get_int("fw_lw_enable_x");
		lw_enable_1 = nvram_get_int("fw_lw_enable_x_1");
		
		if (lw_enable == 1)
			timematch_conv(lw_timematch, "filter_lw_date_x", "filter_lw_time_x");
		if (lw_enable_1 == 1)
			timematch_conv(lw_timematch_1, "filter_lw_date_x", "filter_lw_time_x_1");
		
		if (nvram_match("filter_lw_default_x", "DROP"))
		{
			jtype = logdrop;
			ftype = logaccept;
		}
		else
		{
			jtype = logaccept;
			ftype = logdrop;
		}
		
		foreach_x("filter_lw_num_x")
		{
			g_buf_init();
			
			proto = protoflag_conv("filter_lw_proto_x", i, 0);
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_lw_srcip_x", i);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstip = iprange_ex_conv("filter_lw_dstip_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
			filterstr = filter_conv(proto, flag, srcip, srcport, dstip, dstport);
			
			if (*filterstr)
			{
				if (lw_enable == 1)
					fprintf(fp, "-A %s -i %s %s%s -j %s\n", dtype, lan_if, filterstr, lw_timematch, ftype);
				
				if (lw_enable_1 == 1)
					fprintf(fp, "-A %s -i %s %s%s -j %s\n", dtype, lan_if, filterstr, lw_timematch_1, ftype);
			}
		}
		
		// ICMP
		icmp_ptr[0] = 0;
		foreach(icmp_ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
		{
			if (*icmp_ptr)
			{
				if (lw_enable == 1)
					fprintf(fp, "-A %s -i %s -o %s -p icmp --icmp-type %s%s -j %s\n", dtype, lan_if, wan_if, icmp_ptr, lw_timematch, ftype);
				
				if (lw_enable_1 == 1)
					fprintf(fp, "-A %s -i %s -o %s -p icmp --icmp-type %s%s -j %s\n", dtype, lan_if, wan_if, icmp_ptr, lw_timematch_1, ftype);
			}
		}
		
		// Default
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, jtype);
	}
}

void include_vts_filter(FILE *fp, char *lan_ip, char *logaccept, int forward_chain)
{
	int i;
	char *proto, *protono, *port, *lport, *dstip, *dtype;
	char dstports[16];

	if (forward_chain)
		dtype = "FORWARD";
	else
		dtype = "INPUT";

	foreach_x("vts_num_x")
	{
		g_buf_init();
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (inet_addr_(dstip) == INADDR_ANY)
			continue;
		
		if (forward_chain)
		{
			if (strcmp(lan_ip, dstip) == 0)
				continue;
		}
		else
		{
			if (strcmp(lan_ip, dstip) != 0)
				continue;
		}
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (lport && strlen(lport)!=0)
			sprintf(dstports, "%s", lport);
		else
			sprintf(dstports, "%s", port);
		
		if (*dstports)
		{
			if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s -d %s --dport %s -j %s\n", dtype, "tcp", dstip, dstports, logaccept);
			
			if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
				fprintf(fp, "-A %s -p %s -d %s --dport %s -j %s\n", dtype, "udp", dstip, dstports, logaccept);
		}
		
		if (strcmp(proto, "other")==0)
		{
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s -d %s -j %s\n", dtype, protono, dstip, logaccept);
		}
	}
}

void include_vts_nat(FILE *fp)
{
	int i;
	char *proto, *protono, *port, *lport, *dstip, *dtype;
	
	dtype = "VSERVER";
	
	foreach_x("vts_num_x")
	{
		g_buf_init();
		
		dstip = ip_conv("vts_ipaddr_x", i);
		if (inet_addr_(dstip) == INADDR_ANY)
			continue;
		
		proto = proto_conv("vts_proto_x", i);
		port = portrange_conv("vts_port_x", i);
		lport = portrange_conv("vts_lport_x", i);
		
		if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
		{
			if (lport && strlen(lport)!=0)
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "tcp", port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to %s\n", dtype, "tcp", port, dstip);
		}
		
		if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
		{
			if (lport && strlen(lport)!=0) 
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to-destination %s:%s\n", dtype, "udp", port, dstip, lport);
			else
				fprintf(fp, "-A %s -p %s --dport %s -j DNAT --to %s\n", dtype, "udp", port, dstip);
		}
		
		if (strcmp(proto, "other")==0)
		{
			protono = portrange_conv("vts_protono_x", i);
			if (*protono)
				fprintf(fp, "-A %s -p %s -j DNAT --to %s\n", dtype, protono, dstip);
		}
	}
}

int
filter_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;
	char *ftype, *dtype;
	char lan_class[32];
	int i_mac_filter, is_nat_enabled, is_fw_enabled, wport, need_webstr;
	const char *ipt_file = "/tmp/filter_rules";

	need_webstr = 0;

	if (!(fp=fopen(ipt_file, "w"))) return -1;

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":UPNP - [0:0]\n"
		":maclist - [0:0]\n"
		":doslimit - [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");

	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_fw_enabled = nvram_match("fw_enable_x", "1");

	// MACS chain
	i_mac_filter = include_mac_filter(fp, logdrop);

	// INPUT chain
	dtype = "INPUT";

	/* Accept all traffic from localhost */
	fprintf(fp, "-A %s -i lo -j %s\n", dtype, "ACCEPT");

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j maclist\n", dtype, lan_if);

	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, "ACCEPT");

	/* Accept all traffic from LAN clients */
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");

	/* Pass multicast (all, except udp port 1900) */
	if (nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) {
		fprintf(fp, "-A %s -p 2 -d 224.0.0.0/4 -j %s\n", dtype, logaccept);
		fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 ! --dport 1900 -j %s\n", dtype, "ACCEPT");
	}

	/* DoS attack limits */
	if (nvram_match("fw_dos_x", "1"))
		fprintf(fp, "-A %s -i %s -m state --state NEW -j doslimit\n", dtype, wan_if);

	if (is_fw_enabled)
	{
		if ( is_physical_wan_dhcp() )
			fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, logaccept);
		
		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
			fprintf(fp, "-A %s -p tcp -d %s --dport %s -j %s\n", dtype, lan_ip, nvram_safe_get("http_lanport"), logaccept);
		
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp -d %s --dport %d -j %s\n", dtype, lan_ip, 22, logaccept);
		
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 21, logaccept);
		
		if (nvram_match("trmd_enable", "1") && is_torrent_support())
		{
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = 51413;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1"))
			{
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = 9091;
				fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, wan_if, wport, logaccept);
			}
		}
		
		if (!nvram_match("misc_ping_x", "0"))
		{
			// Pass icmp for ping and udp for traceroute
			fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
			fprintf(fp, "-A %s -p udp --dport 33434:33534 -j %s\n", dtype, logaccept);
		}
		
#if defined (USE_IPV6)
		int ipv6_type = get_ipv6_type();
		if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
		{
			if (ipv6_type == IPV6_6IN4 && nvram_match("misc_ping_x", "0"))
			{
				char *tun_remote = nvram_safe_get("ip6_6in4_remote");
				if (*tun_remote)
					fprintf(fp, "-A %s -p icmp -s %s -j %s\n", dtype, tun_remote, logaccept);
			}
			fprintf(fp, "-A %s -p 41 -j %s\n", dtype, logaccept);
		}
#endif
		if (nvram_match("vpns_enable", "1")) 
		{
			if (nvram_match("vpns_type", "1"))
			{
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, 1701, logaccept);
			}
			else
			{
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 1723, logaccept);
				fprintf(fp, "-A %s -p 47 -j %s\n", dtype, logaccept);
			}
			
			fprintf(fp, "-A %s -i ppp+ -s %s -j %s\n", dtype, lan_class, logaccept);
		}
		
		// add Virtual Server rules for router host
		if (is_nat_enabled && nvram_match("vts_enable_x", "1"))
		{
			include_vts_filter(fp, lan_ip, logaccept, 0);
		}
		
		fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD section
	dtype = "FORWARD";
	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j maclist\n", dtype, lan_if);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, logaccept);

	/* Drop all packets in the INVALID state */
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");

	/* Pass multicast */
	if (nvram_match("mr_enable_x", "1"))
		fprintf(fp, "-A %s -p udp -d 224.0.0.0/4 -j %s\n", dtype, "ACCEPT");

	/* use url filter before accepting ESTABLISHED packets */
	if (include_webstr_filter(fp))
		need_webstr = 1;

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets */
	if (nvram_match("wan_proto", "pptp") || nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "l2tp") || get_usb_modem_state())
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, "ACCEPT");

	/* Pass VPN server's clients traffic */
	if (nvram_match("vpns_enable", "1")) 
		fprintf(fp, "-A %s -i ppp+ -s %s -j %s\n", dtype, lan_class, logaccept);

	/* Filter out invalid WAN->WAN connections */
	fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
	if (!nvram_match("wan0_ifname", wan_if))
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, nvram_safe_get("wan0_ifname"), lan_if, logdrop);

	/* DoS attacks */
	if (nvram_match("fw_dos_x", "1"))
		fprintf(fp, "-A %s -i %s -m state --state NEW -j doslimit\n", dtype, wan_if);

	// Block VPN LAN to WAN traffic
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-A %s -i %s -p 47 -j %s\n", dtype, lan_if, logdrop);
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-A %s -i %s -p 50 -j %s\n", dtype, lan_if, logdrop);
		fprintf(fp, "-A %s -i %s -p 51 -j %s\n", dtype, lan_if, logdrop);
	}
	if (nvram_match("fw_pt_pptp", "0"))
		fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, lan_if, 1723, logdrop);
	if (nvram_match("fw_pt_l2tp", "0"))
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 1701, logdrop);
	if (nvram_match("fw_pt_ipsec", "0"))
	{
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 500, logdrop);
		fprintf(fp, "-A %s -i %s -p udp --dport %d -j %s\n", dtype, lan_if, 4500, logdrop);
	}

	// FILTER from LAN to WAN
	include_lw_filter(fp, lan_if, wan_if, logaccept, logdrop);

#if defined (USE_KERNEL3X)
	/* Port trigger */
	if (is_nat_enabled && nvram_match("autofw_enable_x", "1"))
		include_port_trigger(fp, wan_if);
#endif
	if (is_fw_enabled)
	{
		if (is_nat_enabled)
		{
#if defined (USE_KERNEL3X)
			/* Accepts all DNATed connection - BattleNET, DMZ, VSERVER, UPNP, DMZ */
			fprintf(fp, "-A %s -m conntrack --ctstate DNAT -j %s\n", dtype, logaccept);
#else
			/* Accept to BattleNET */
			if (nvram_match("sp_battle_ips", "1"))
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, BATTLENET_PORT, logaccept);
			
			/* Accept to exposed station (DMZ) */
			char *dmz_ip = nvram_safe_get("dmz_ip");
			if (inet_addr_(dmz_ip))
				fprintf(fp, "-A %s -d %s -j %s\n", dtype, dmz_ip, logaccept);
			
			/* Accept to Virtual Servers */
			if (nvram_match("vts_enable_x", "1"))
			{
				include_vts_filter(fp, lan_ip, logaccept, 1);
			}
			
			/* Jump to IGD UPnP/NAT-PMP (miniupnpd chain) */
			fprintf(fp, "-A %s -j UPNP\n", dtype);
#endif
		}
		
		/* Default forward rule (drop all packets -> LAN) */
		fprintf(fp, "-A %s -o %s -j %s\n", dtype, lan_if, logdrop);
	}

	// doslimit chain
	dtype = "doslimit";
	ftype = "RETURN";
	fprintf(fp, "-A %s -p tcp --syn -m limit --limit 20/s --limit-burst 30 -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -p tcp --syn -j %s\n", dtype, logdrop);
	fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -m limit --limit 1/s -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -p tcp --tcp-flags SYN,ACK,FIN,RST RST -j %s\n", dtype, logdrop);
	fprintf(fp, "-A %s -p icmp --icmp-type echo-request -m limit --limit 2/s -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -p icmp --icmp-type echo-request -j %s\n", dtype, logdrop);

	// logaccept chain
	dtype = "logaccept";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	// logdrop chain
	dtype = "logdrop";
	ftype = "DROP";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
	
	if (need_webstr)
		system("modprobe -q xt_webstr");
	
	doSystem("iptables-restore %s", ipt_file);
	
	if (!need_webstr)
		system("modprobe -r xt_webstr");
	
	return 0;
}

void
default_filter_setting(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/filter.default";

	if (nvram_invmatch("wan_route_x", "IP_Routed")) return;

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if ((fp=fopen(ipt_file, "w"))==NULL) return;

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":UPNP - [0:0]\n"
		":maclist - [0:0]\n"
		":doslimit - [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");

	// INPUT chain
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -i lo -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);

	if (is_fw_enabled)
	{
		fprintf(fp, "-A %s -p udp --sport %d --dport %d -j %s\n", dtype, 67, 68, ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, "DROP");
	}

	// FORWARD chain
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, ftype);

	if (is_fw_enabled)
		fprintf(fp, "-A %s -o %s -j %s\n", dtype, IFNAME_BR, "DROP");

	// logaccept chain
	dtype = "logaccept";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	// logdrop chain
	dtype = "logdrop";
	ftype = "DROP";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("iptables-restore %s", ipt_file);
}

#if defined (USE_IPV6)
int
filter6_setting(char *wan_if, char *lan_if, char *logaccept, char *logdrop)
{
	FILE *fp;
	char *ftype, *dtype;
	int i_mac_filter, is_fw_enabled, wport, ipv6_type;
	const char *ipt_file = "/tmp/filter6_rules";

	ipv6_type = get_ipv6_type();
	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if (!(fp=fopen(ipt_file, "w"))) return -1;

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":maclist - [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");

	// MACS chain
	i_mac_filter = include_mac_filter(fp, logdrop);

	// INPUT chain
	dtype = "INPUT";

	/* Accept all traffic from localhost */
	fprintf(fp, "-A %s -i lo -j %s\n", dtype, "ACCEPT");

	/* Policy for all traffic from MAC-filtered LAN clients */
	if (i_mac_filter > 0 && nvram_match("fw_mac_drop", "1"))
		fprintf(fp, "-A %s -i %s -j maclist\n", dtype, lan_if);

	/* Allow ICMPv6 */
	if (!is_fw_enabled || !nvram_match("misc_ping_x", "0"))
		fprintf(fp, "-A %s -p 58 -j %s\n", dtype, logaccept);
	else
		fprintf(fp, "-A %s -p 58 ! --icmpv6-type echo-request -j %s\n", dtype, logaccept);

	// Disable processing of any RH0 packet
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, logdrop);

	/* Drop the wrong state, INVALID, packets */
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, "ACCEPT");

	/* Accept all traffic from LAN clients */
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, "ACCEPT");

	/* Accept all Link-Local addresses */
	fprintf(fp, "-A %s -s fe80::/10 -j %s\n", dtype, logaccept);

	/* Accept all multicast */
	fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, "ACCEPT");

	if (is_fw_enabled)
	{
		/* Allow DHCPv6 */
		if (ipv6_type == IPV6_NATIVE_DHCP6)
			fprintf(fp, "-A %s -p udp --dport 546 -j %s\n", dtype, logaccept);
		
		// Firewall between WAN and Local

/*		http/ssh disabled from wan yet (no NAT in IPv6)
		if (nvram_match("misc_http_x", "1"))
			fprintf(fp, "-A %s -p tcp --dport %s -j %s\n", dtype, nvram_safe_get("http_lanport"), logaccept);
		
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 22, logaccept);
*/
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 21, logaccept);
		
		if (nvram_match("trmd_enable", "1") && is_torrent_support())
		{
			wport = nvram_get_int("trmd_pport");
			if (wport < 1024 || wport > 65535) wport = 51413;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1"))
			{
				wport = nvram_get_int("trmd_rport");
				if (wport < 1024 || wport > 65535) wport = 9091;
				fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, wan_if, wport, logaccept);
			}
		}
		
		if (!nvram_match("misc_ping_x", "0"))
		{
			// Pass udp for traceroute
			fprintf(fp, "-A %s -p udp --dport 33434:33534 -j %s\n", dtype, logaccept);
		}
		
		if (nvram_match("vpns_enable", "1")) 
		{
			if (nvram_match("vpns_type", "1"))
			{
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, 1701, logaccept);
			}
			else
			{
				fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 1723, logaccept);
				fprintf(fp, "-A %s -p 47 -j %s\n", dtype, logaccept);
			}
		}
		
		fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

	// FORWARD chain
	dtype = "FORWARD";

	if (i_mac_filter > 0)
		fprintf(fp, "-A %s -i %s -j maclist\n", dtype, lan_if);

	/* Accept the redirect, might be seen as INVALID, packets */
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, lan_if, lan_if, logaccept);

	// Allow ICMPv6
	fprintf(fp, "-A %s -p 58 -j %s\n", dtype, logaccept);

	// Disable processing of any RH0 packet
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, logdrop);

	/* Drop all packets in the INVALID state */
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");

	/* Clamp TCP MSS to PMTU of WAN interface before accepting RELATED packets  */
	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
		fprintf(fp, "-A %s -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n", dtype);

	/* Accept related connections, skip rest of checks */
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, "ACCEPT");

	/* Filter out invalid WAN->WAN connections */
	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, IFNAME_SIT, lan_if, logdrop);
	fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, wan_if, lan_if, logdrop);
	if (!nvram_match("wan0_ifname", wan_if))
		fprintf(fp, "-A %s -o %s ! -i %s -j %s\n", dtype, nvram_safe_get("wan0_ifname"), lan_if, logdrop);

	// INPUT chain
	dtype = "OUTPUT";

	// Disable processing of any RH0 packet
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, logdrop);

	// logaccept chain
	dtype = "logaccept";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	// logdrop chain
	dtype = "logdrop";
	ftype = "DROP";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
	
	return doSystem("ip6tables-restore %s", ipt_file);
}

void
default_filter6_setting(void)
{
	FILE *fp;
	char *ftype, *dtype;
	int is_fw_enabled;
	const char *ipt_file = "/tmp/filter6.default";

	if (nvram_invmatch("wan_route_x", "IP_Routed")) return;

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	if ((fp=fopen(ipt_file, "w"))==NULL) return;

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":maclist - [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");

	// INPUT chain
	dtype = "INPUT";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -i lo -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -p 58 ! --icmpv6-type echo-request -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -s fe80::/10 -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -d ff00::/8 -j %s\n", dtype, ftype);

	if (is_fw_enabled)
	{
		fprintf(fp, "-A %s -p udp --dport 546 -j %s\n", dtype, ftype);
		fprintf(fp, "-A %s -j %s\n", dtype, "DROP");
	}

	// FORWARD chain
	dtype = "FORWARD";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -i %s -o %s -j %s\n", dtype, IFNAME_BR, IFNAME_BR, ftype);
	fprintf(fp, "-A %s -p 58 -j %s\n", dtype, ftype);
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state INVALID -j %s\n", dtype, "DROP");
	fprintf(fp, "-A %s -m state --state ESTABLISHED,RELATED -j %s\n", dtype, ftype);

	// INPUT chain
	dtype = "OUTPUT";
	fprintf(fp, "-A %s -m rt --rt-type 0 -j %s\n", dtype, "DROP");

	// logaccept chain
	dtype = "logaccept";
	ftype = "ACCEPT";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	// logdrop chain
	dtype = "logdrop";
	ftype = "DROP";
	fprintf(fp, "-A %s -m state --state NEW -j LOG --log-prefix \"%s \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n", dtype, ftype);
	fprintf(fp, "-A %s -j %s\n", dtype, ftype);

	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	doSystem("ip6tables-restore %s", ipt_file);
}
#endif


void nat_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip)
{
	FILE *fp;
	int wport, is_nat_enabled, is_fw_enabled, use_battlenet;
	char dmz_ip[32], lan_class[32];
	char *wanx_ipaddr = NULL;
	const char *ipt_file = "/tmp/nat_rules";
	
	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_fw_enabled = nvram_match("fw_enable_x", "1");
	
	if (nvram_invmatch("wan0_proto", "static") && nvram_invmatch("wan0_ifname", wan_if) && inet_addr_(nvram_safe_get("wanx_ipaddr")))
		wanx_ipaddr = nvram_safe_get("wanx_ipaddr");
	
	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
	
	if ((fp=fopen(ipt_file, "w"))==NULL) return;	// oleg patch
	
	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":VSERVER - [0:0]\n"
		":UPNP - [0:0]\n");
		
	if (inet_addr_(wan_ip))
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wan_ip);
	
	if (wanx_ipaddr)
		fprintf(fp, "-A PREROUTING -d %s -j VSERVER\n", wanx_ipaddr);
	
	if (is_nat_enabled)
	{
		strcpy(dmz_ip, nvram_safe_get("dmz_ip"));
		
		/* BattleNET (PREROUTING) */
		use_battlenet = (nvram_match("sp_battle_ips", "1") && inet_addr_(wan_ip)) ? 1 : 0;
		if (use_battlenet)
			fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BATTLENET_PORT, lan_class);
		
#if !defined (USE_KERNEL3X)
		/* Port trigger (PREROUTING) */
		if (nvram_match("autofw_enable_x", "1"))
			include_porttrigger_preroute(fp, lan_if);
#endif
		/* BattleNET (POSTROUTING) */
		if (use_battlenet)
			fprintf(fp, "-A POSTROUTING -p udp -s %s --dport %d -j NETMAP --to %s\n", lan_class, BATTLENET_PORT, wan_ip);
		
		/* use SNAT instead of MASQUERADE (more fast) */
		/* masquerade WAN connection */
		if (inet_addr_(wan_ip))
			fprintf(fp, "-A POSTROUTING -o %s -s %s -j SNAT --to-source %s\n", 
				wan_if, lan_class, wan_ip);
		
		/* masquerade physical WAN port connection */
		if (wanx_ipaddr)
			fprintf(fp, "-A POSTROUTING -o %s -s %s -j SNAT --to-source %s\n", 
				nvram_safe_get("wan0_ifname"), lan_class, wanx_ipaddr);
		
		// masquerade lan to lan (NAT loopback)
		if (nvram_match("nf_nat_loop", "1"))
			fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n", 
				lan_if, lan_class, lan_class, lan_ip);
		
		/* Local ports remap (http and ssh) */
		if (is_fw_enabled)
		{
			if (nvram_match("misc_http_x", "1"))
			{
				wport = nvram_get_int("misc_httpport_x");
				if (wport < 80 || wport > 65535) wport = 8080;
				fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s:%s\n",
					wport, lan_ip, nvram_safe_get("http_lanport"));
			}
			
			if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1"))
			{
				wport = nvram_get_int("sshd_wport");
				if (wport < 22 || wport > 65535) wport = 10022;
				fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s:%d\n",
					wport, lan_ip, 22);
			}
		}
		
		/* check DMZ host is set, pre-route several traffic to router local first */
		if (inet_addr_(dmz_ip))
		{
			/* pre-route for local VPN server */
			if (nvram_match("vpns_enable", "1"))
			{
				if (nvram_match("vpns_type", "1"))
				{
					fprintf(fp, "-A VSERVER -p udp --dport %d -j DNAT --to-destination %s\n", 1701, lan_ip);
				}
				else
				{
					fprintf(fp, "-A VSERVER -p 47 -j DNAT --to-destination %s\n", lan_ip);
					fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s\n", 1723, lan_ip);
				}
			}
			
			/* pre-route for local Transmission */
			if (nvram_match("trmd_enable", "1") && is_torrent_support())
			{
				wport = nvram_get_int("trmd_pport");
				if (wport < 1024 || wport > 65535) wport = 51413;
				fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				fprintf(fp, "-A VSERVER -p udp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				
				if (nvram_match("trmd_ropen", "1"))
				{
					wport = nvram_get_int("trmd_rport");
					if (wport < 1024 || wport > 65535) wport = 9091;
					fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				}
			}
		}
		
		/* Virtual Server mappings */
		if (nvram_match("vts_enable_x", "1"))
		{
			include_vts_nat(fp);
		}
		
#if defined (USE_KERNEL3X)
		/* Port trigger*/
		if (nvram_match("autofw_enable_x", "1"))
			fprintf(fp, "-A VSERVER -j TRIGGER --trigger-type dnat\n");
#endif
		/* IGD UPnP */
		if (nvram_invmatch("upnp_enable", "0"))
			fprintf(fp, "-A VSERVER -j UPNP\n");
		
		/* Exposed station (DMZ) */
		if (inet_addr_(dmz_ip))
			fprintf(fp, "-A VSERVER -j DNAT --to %s\n", dmz_ip);
	}
	
	fprintf(fp, "COMMIT\n\n");
	
	fclose(fp);
	
	doSystem("iptables-restore %s", ipt_file);
}

void
default_nat_setting(void)
{
	FILE *fp;
	char* lan_ip;
	char lan_class[32];
	const char *ipt_file = "/tmp/nat.default";
	
	if (nvram_invmatch("wan_nat_x", "1")) return;
	
	if ((fp=fopen(ipt_file, "w"))==NULL) return;
	
	fprintf(fp, "*nat\n"
		":PREROUTING ACCEPT [0:0]\n"
		":POSTROUTING ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"	// oleg patch
		":VSERVER - [0:0]\n"		// oleg patch
		":UPNP - [0:0]\n");
	
	if (nvram_invmatch("upnp_enable", "0"))
	{
		/* Call UPNP chain */
		fprintf(fp, "-A VSERVER -j UPNP\n");
	}
	
	/* use SNAT instead of MASQUERADE (more fast) */
	
	// masquerade lan to lan (NAT loopback)
	if (nvram_match("nf_nat_loop", "1")) 
	{
		lan_ip = nvram_safe_get("lan_ipaddr");
		ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
		fprintf(fp, "-A POSTROUTING -o %s -s %s -d %s -j SNAT --to-source %s\n", 
			IFNAME_BR, lan_class, lan_class, lan_ip);
	}
	
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
	
	doSystem("iptables-restore %s", ipt_file);
}


int
start_firewall_ex(char *wan_if, char *wan_ip)
{
	FILE *fp;
	int i_nf_nat, i_nf_val;
	char rp_path[64], logaccept[32], logdrop[32];
	char *lan_if, *lan_ip;
	char *opt_iptables_script = "/opt/bin/update_iptables.sh";
	char *int_iptables_script = "/etc/storage/post_iptables_script.sh";
	
	if (is_ap_mode())
		return -1;
	
	lan_if = IFNAME_BR;
	lan_ip = nvram_safe_get("lan_ipaddr");
	
	/* mcast needs rp filter to be turned off only for non default iface */
	sprintf(rp_path, "/proc/sys/net/ipv4/conf/%s/rp_filter", IFNAME_WAN);
	if (!(nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) || (strcmp(wan_if, IFNAME_WAN) == 0)) 
		fput_int(rp_path, 1);
	else
		fput_int(rp_path, 0);
	
	/* Determine the log type */
	if (nvram_match("fw_log_x", "accept") || nvram_match("fw_log_x", "both"))
		strcpy(logaccept, "logaccept");
	else
		strcpy(logaccept, "ACCEPT");

	if (nvram_match("fw_log_x", "drop") || nvram_match("fw_log_x", "both"))
		strcpy(logdrop, "logdrop");
	else
		strcpy(logdrop, "DROP");

	/* NAT setting */
	nat_setting(wan_if, wan_ip, lan_if, lan_ip);

	/* Filter setting */
	filter_setting(wan_if, wan_ip, lan_if, lan_ip, logaccept, logdrop);
#if defined (USE_IPV6)
	if (get_ipv6_type() != IPV6_DISABLED)
		filter6_setting(wan_if, lan_if, logaccept, logdrop);
	else
		default_filter6_setting();
#endif
	i_nf_val = nvram_get_int("nf_nat_type");
	if (i_nf_val == 2)
		i_nf_nat = 0;	// Linux
	else if (i_nf_val == 1)
		i_nf_nat = 1;	// FCONE
	else
		i_nf_nat = 2;	// RCONE
	fput_int("/proc/sys/net/nf_conntrack_nat_mode", i_nf_nat);

	i_nf_val = nvram_get_int("nf_max_conn");
	if (i_nf_val < 8192) i_nf_val = 8192;
	if (i_nf_val > 262144) i_nf_val = 262144;
	fput_int("/proc/sys/net/nf_conntrack_max", i_nf_val);
#if !defined (USE_KERNEL3X)
	fput_int("/proc/sys/net/nf_conntrack_max_general", i_nf_val - 384);
	fput_int("/proc/sys/net/nf_conntrack_fastnat", is_fastnat_allow());
#endif
	/* Tweak NAT performance... */
	fput_int("/proc/sys/net/ipv4/tcp_fin_timeout", 40);
	fput_int("/proc/sys/net/ipv4/tcp_keepalive_intvl", 30);
	fput_int("/proc/sys/net/ipv4/tcp_keepalive_probes", 5);
	fput_int("/proc/sys/net/ipv4/tcp_keepalive_time", 1800);
	fput_int("/proc/sys/net/ipv4/tcp_retries2", 5);
	fput_int("/proc/sys/net/ipv4/tcp_syn_retries", 3);
	fput_int("/proc/sys/net/ipv4/tcp_synack_retries", 3);
	fput_int("/proc/sys/net/ipv4/tcp_tw_recycle", 1);
	fput_int("/proc/sys/net/ipv4/tcp_tw_reuse", 1);

	/* Tweak DoS-related... */
	fput_int("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", 1);
	fput_int("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", 1);
	fput_int("/proc/sys/net/ipv4/tcp_rfc1337", 1);
	fput_int("/proc/sys/net/ipv4/tcp_syncookies", nvram_get_int("fw_syn_cook"));
	
	/* Enable IPv4/IPv6 forwarding */
	set_ip_forward();
	
	if (check_if_file_exist(int_iptables_script))
		doSystem("%s", int_iptables_script);
	
	if (check_if_file_exist(opt_iptables_script))
		doSystem("%s update", opt_iptables_script);
	
	return 0;
}

