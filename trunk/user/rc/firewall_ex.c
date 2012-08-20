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
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
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

#define foreach_x(x)	for (i=0; i<atoi(nvram_safe_get(x)); i++)
typedef unsigned char bool;	// 1204 ham


#define BATTLENET_PORT 6112

char *g_buf;
char g_buf_pool[1024];

// 0816 mv
bool
valid_autofw_port(const netconf_app_t *app)
{
	/* Check outbound protocol */
	if (app->match.ipproto != IPPROTO_TCP && app->match.ipproto != IPPROTO_UDP)
		return FALSE;

	/* Check outbound port range */
	if (ntohs(app->match.dst.ports[0]) > ntohs(app->match.dst.ports[1]))
		return FALSE;

	/* Check related protocol */
	if (app->proto != IPPROTO_TCP && app->proto != IPPROTO_UDP)
		return FALSE;

	/* Check related destination port range */
	if (ntohs(app->dport[0]) > ntohs(app->dport[1]))
		return FALSE;

	/* Check mapped destination port range */
	if (ntohs(app->to[0]) > ntohs(app->to[1]))
		return FALSE;

	/* Check port range size */
	if ((ntohs(app->dport[1]) - ntohs(app->dport[0])) !=
	    (ntohs(app->to[1]) - ntohs(app->to[0])))
		return FALSE;
	return TRUE;
}

void g_buf_init()
{
	g_buf = g_buf_pool;
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
		//sprintf(g_buf, "");
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
		//sprintf(newstr, " --tcp-flags %s RST", flag);
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
	char *datestr[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char timestart[6], timestop[6];
	char *time, *date;
	int i, head;

	date = nvram_get(nv_date);
	time = nvram_get(nv_time);

	if ((!date) || strlen(date) != 7 || !strcmp(date, "0000000") || !time || strlen(time) != 8)
		goto no_match;

	if (strncmp(date, "1111111", 7) == 0 &&
	    strncmp(time, "00002359", 8) == 0) goto no_match;

	i=0;
	strncpy(timestart, time, 2);
	i+=2;
	timestart[i++] = ':';
	strncpy(timestart+i, time+2, 2);
	i+=2;
	timestart[i]=0;
	i=0;
	strncpy(timestop, time+4, 2);
	i+=2;
	timestop[i++] = ':';
	strncpy(timestop+i, time+6, 2);
	i+=2;
	timestop[i]=0;

	sprintf(mstr, "-m time --timestart %s:00 --timestop %s:00 --days ",
//	sprintf(mstr, "-m time --timestart %s --timestop %s --days ",
			timestart, timestop);

	head=1;

	for (i=0;i<7;i++)
	{
		if (*(date+i)=='1')
		{
			if (head)
			{
				sprintf(mstr, "%s %s", mstr, datestr[i]);
				head=0;
			}
			else
			{	
				sprintf(mstr, "%s,%s", mstr, datestr[i]);
			}
		}
	}
	return;
	
no_match:
	//sprintf(mstr, "");
	mstr[0] = 0;	// oleg patch
	return;
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

void
p(int step)
{
	dprintf("P: %d %s\n", step, g_buf);
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
/*
	in.s_addr = ip & val;
	if (val==0xff00000) sprintf(buf, "%s/8", inet_ntoa(in));
	else if (val==0xffff0000) sprintf(buf, "%s/16", inet_ntoa(in));
	else sprintf(buf, "%s/24", inet_ntoa(in));
*/
	// oleg patch ~
	in.s_addr = ip & val;

	for (val = ntohl(val); val; i++) 
		val <<= 1;

	sprintf(buf, "%s/%d", inet_ntoa(in), i);
	// ~ oleg patch
	dprintf(buf);	
}

void convert_routes(void)
{
	int i;
	char *ip, *netmask, *gateway, *matric, *interface;
	char wroutes[1024], lroutes[1024], mroutes[2048];	// oleg patch

	wroutes[0] = 0;
	lroutes[0] = 0;	
	mroutes[0] = 0;	// oleg patch

	g_buf_init();
	
	if (nvram_match("sr_enable_x", "1")) foreach_x("sr_num_x")	// oleg patch
	{
		ip = general_conv("sr_ipaddr_x", i);
		netmask = general_conv("sr_netmask_x", i);
		gateway = general_conv("sr_gateway_x", i);
		matric = general_conv("sr_matric_x", i);
		interface = general_conv("sr_if_x", i);

		if (!strcmp(interface, "WAN"))
		{
			sprintf(wroutes, "%s %s:%s:%s:%d", wroutes, ip, netmask, gateway, atoi(matric)+1);
		}
		else if (!strcmp(interface, "MAN"))	// oleg patch
		{
			sprintf(mroutes, "%s %s:%s:%s:%d", mroutes, ip, netmask, gateway, atoi(matric)+1);
		} 
		else if (!strcmp(interface, "LAN"))
		{
			sprintf(lroutes, "%s %s:%s:%s:%d", lroutes, ip, netmask, gateway, atoi(matric)+1);
		}
	}

	nvram_set("lan_route", lroutes);
	nvram_set("wan0_route", wroutes);
	nvram_set("wan_route", mroutes);	// oleg patch
}

int include_porttrigger_preroute(FILE *fp, char *lan_if)
{
	netconf_app_t apptarget, *app;
	int i;
	char *out_proto, *in_proto, *out_port, *in_port, *desc;
	int  out_start, out_end, in_start, in_end, use_autofw;
	
	use_autofw = 0;
	
	g_buf_init();
	foreach_x("autofw_num_x")
	{
		out_proto = proto_conv("autofw_outproto_x", i);
		out_port = portrange_ex2_conv_new("autofw_outport_x", i, &out_start, &out_end);
		in_proto = proto_conv("autofw_inproto_x", i);
		in_port = portrange_ex2_conv("autofw_inport_x", i, &in_start, &in_end);
		desc = general_conv("autofw_desc_x", i);
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

		/* Parse description */
		if (desc)
			strncpy(app->desc, desc, sizeof(app->desc));

		/* Set interface name (match packets entering LAN interface) */
		strncpy(app->match.in.name, lan_if, IFNAMSIZ);

		/* Set LAN source port range (match packets from any source port) */
		app->match.src.ports[1] = htons(0xffff);

		/* Set target (application specific port forward) */
		app->target = NETCONF_APP;

		if (valid_autofw_port(app))
		{
			/* cmd format:
			 * iptables -t nat -A PREROUTING -i br0 -p INCOMING_PROTOCOL --dport TRIGGER_PORT_FROM(-TRIGGER_PORT_TO) -j autofw --related-proto TRIGGER_PROTOCOL --related-dport INCOMING_PORT_FROM(-INCOMING_PORT_TO) --related-to INCOMING_PORT_FROM(-INCOMING_PORT_TO)
			 *
			 * For example, to set up a trigger for BitTorrent, you'd use this:
			 * iptables -t nat -A PREROUTING -i br0 -p tcp --dport 6881 -j autofw --related-proto tcp --related-dport 6881-6999 --related-to 6881-6999
			 */
			fprintf(fp, "-A PREROUTING -i %s -p %s --dport %s -j autofw --related-proto %s --related-dport %s --related-to %s\n",
				lan_if,
				out_proto,
				out_port,
				in_proto,
				in_port,
				in_port);
			
			use_autofw = 1;
		}
	}
	
	return use_autofw;
}

void nat_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip)
{
	FILE *fp;
	int i, need_netmap, need_autofw, wport, is_nat_enabled, is_fw_enabled, use_battlenet;
	char dmz_ip[32], dstips[32], dstports[32], lan_class[32];
	char *wanx_ipaddr = NULL;
	
	need_netmap = 0;
	need_autofw = 0;
	is_nat_enabled = nvram_match("wan_nat_x", "1");
	is_fw_enabled = nvram_match("fw_enable_x", "1");
	
	if (nvram_invmatch("wan0_proto", "static") && nvram_invmatch("wan0_ifname", wan_if) && inet_addr_(nvram_safe_get("wanx_ipaddr")))
		wanx_ipaddr = nvram_safe_get("wanx_ipaddr");
	
	ip2class(lan_ip, nvram_safe_get("lan_netmask"), lan_class);
	
	if ((fp=fopen("/tmp/nat_rules", "w"))==NULL) return;	// oleg patch
	
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
		{
			fprintf(fp, "-A PREROUTING -p udp -d %s --sport %d -j NETMAP --to %s\n", wan_ip, BATTLENET_PORT, lan_class);
			need_netmap = 1;
		}
		
		/* Port trigger (PREROUTING) */
		if (nvram_match("autofw_enable_x", "1"))
		{
			if ( include_porttrigger_preroute(fp, lan_if) )
				need_autofw = 1;
		}
		
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
				wport=atoi(nvram_safe_get("misc_httpport_x"));
				if (wport < 80 || wport > 65535) wport = 8080;
				fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s:%s\n",
					wport, lan_ip, nvram_safe_get("http_lanport"));
			}
			
			if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1"))
			{
				wport=atoi(nvram_safe_get("sshd_wport"));
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
				wport=atoi(nvram_safe_get("trmd_pport"));
				if (wport < 1024 || wport > 65535) wport = 51413;
				fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				fprintf(fp, "-A VSERVER -p udp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				
				if (nvram_match("trmd_ropen", "1"))
				{
					wport=atoi(nvram_safe_get("trmd_rport"));
					if (wport < 1024 || wport > 65535) wport = 9091;
					fprintf(fp, "-A VSERVER -p tcp --dport %d -j DNAT --to-destination %s\n", wport, lan_ip);
				}
			}
		}
		
		/* Virtual Server mappings */
		if (nvram_match("vts_enable_x", "1"))
		{
			g_buf_init();
			foreach_x("vts_num_x")
			{
				char *proto, *protono, *port, *lport, *dstip;
				
				proto = proto_conv("vts_proto_x", i);
				protono = portrange_conv("vts_protono_x", i);
				port = portrange_conv("vts_port_x", i);
				lport = portrange_conv("vts_lport_x", i);
				dstip = ip_conv("vts_ipaddr_x", i);
				
				if (lport && strlen(lport)!=0) 
				{
					sprintf(dstips, "%s:%s", dstip, lport);
					sprintf(dstports, "%s", lport);
				}
				else
				{
					sprintf(dstips, "%s:%s", dstip, port);
					sprintf(dstports, "%s", port);
				}
				
				if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
				{
					if (lport && strlen(lport)!=0) 
						fprintf(fp, "-A VSERVER -p tcp --dport %s -j DNAT --to-destination %s\n", port, dstips);
					else
						fprintf(fp, "-A VSERVER -p tcp --dport %s -j DNAT --to %s\n", port, dstip);
				}
				
				if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
				{
					if (lport && strlen(lport)!=0) 
						fprintf(fp, "-A VSERVER -p udp --dport %s -j DNAT --to-destination %s\n", port, dstips);
					else
						fprintf(fp, "-A VSERVER -p udp --dport %s -j DNAT --to %s\n", port, dstip);
				}
				
				if (strcmp(proto, "other")==0)
					fprintf(fp, "-A VSERVER -p %s -j DNAT --to %s\n", protono, dstip);
			}
		}
		
		/* IGD UPnP */
		if (nvram_invmatch("upnp_enable", "0"))
			fprintf(fp, "-A VSERVER -j UPNP\n");
		
		/* Exposed station (DMZ) */
		if (inet_addr_(dmz_ip))
			fprintf(fp, "-A VSERVER -j DNAT --to %s\n", dmz_ip);
	}
	
	fprintf(fp, "COMMIT\n\n");
	
	fclose(fp);
	
	if (need_netmap)
		system("modprobe -q ipt_NETMAP");
	
	if (need_autofw)
		system("modprobe -q ipt_autofw");
	
	system("iptables-restore /tmp/nat_rules");
	
	if (!need_netmap)
		system("modprobe -r ipt_NETMAP");
	
	if (!need_autofw)
		system("modprobe -r ipt_autofw");
}


void
default_nat_setting(void)
{
	FILE *fp;
	char* lan_ip;
	char lan_class[32];
	
	if (nvram_invmatch("wan_nat_x", "1")) return;
	
	if ((fp=fopen("/tmp/nat.default", "w"))==NULL) return;
	
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
	
	system("iptables-restore /tmp/nat.default");
}


int makeTimestr(char *tf)
{
	char *url_time = nvram_get("url_time_x");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000") || !url_time)
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	return 0;
}

int makeTimestr2(char *tf)
{
	char *url_time = nvram_get("url_time_x_1");
	char *url_date = nvram_get("url_date_x");
	static const char *days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	int i, comma = 0;

	memset(tf, 0, 256);

	if (!nvram_match("url_enable_x_1", "1"))
		return -1;

	if ((!url_date) || strlen(url_date) != 7 || !strcmp(url_date, "0000000") || !url_time)
	{
		printf("url filter get time fail\n");
		return -1;
	}

	sprintf(tf, "-m time --timestart %c%c:%c%c:00 --timestop %c%c:%c%c:59 --days ", url_time[0], url_time[1], url_time[2], url_time[3], url_time[4], url_time[5], url_time[6], url_time[7]);

	for (i=0; i<7; ++i)
	{
		if (url_date[i] == '1')
		{
			if (comma == 1)
				strncat(tf, ",", 1);

			strncat(tf, days[i], 3);
			comma = 1;
		}
	}

	return 0;
}

int valid_url_filter_time()
{
	char *url_time1 = nvram_get("url_time_x");
	char *url_time2 = nvram_get("url_time_x_1");
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

	if (!nvram_match("url_enable_x", "1") && !nvram_match("url_enable_x_1", "1"))
		return 0;

	if (nvram_match("url_enable_x", "1"))
	{
		if ((!url_time1) || strlen(url_time1) != 8)
			goto err;

		strncpy(starttime1, url_time1, 4);
		strncpy(endtime1, url_time1 + 4, 4);

		if (atoi(starttime1) >= atoi(endtime1))
			goto err;
	}

	if (nvram_match("url_enable_x_1", "1"))
	{
		if ((!url_time2) || strlen(url_time2) != 8)
			goto err;

		strncpy(starttime2, url_time2, 4);
		strncpy(endtime2, url_time2 + 4, 4);

		if (atoi(starttime2) >= atoi(endtime2))
			goto err;
	}

	if (nvram_match("url_enable_x", "1") && nvram_match("url_enable_x_1", "1"))
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
	printf("invalid url filter time setting!\n");
	return 0;
}


int include_webstr_filter(FILE *fp)
{
	int i, use_webstr;
	char nvname[36], timef[256], timef2[256], *filterstr, *dtype;
	
	use_webstr = 0;
	
	dtype = "FORWARD";
	
	if (valid_url_filter_time())
	{
		if (!makeTimestr(timef))
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
			{
				sprintf(nvname, "url_keyword_x%d", i);
				filterstr = nvram_safe_get(nvname);
				if (strncasecmp(filterstr, "http://", 7) == 0)
					filterstr += 7;
				else if (strncasecmp(filterstr, "https://", 8) == 0)
					filterstr += 8;
				if (*filterstr)
				{
					fprintf(fp,"-A %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, timef, filterstr);
					use_webstr = 1;
				}
			}
		
		if (!makeTimestr2(timef2))
			for (i=0; i<atoi(nvram_safe_get("url_num_x")); i++)
			{
				sprintf(nvname, "url_keyword_x%d", i);
				filterstr = nvram_safe_get(nvname);
				if (strncasecmp(filterstr, "http://", 7) == 0)
					filterstr += 7;
				else if (strncasecmp(filterstr, "https://", 8) == 0)
					filterstr += 8;
				if (*filterstr)
				{
					fprintf(fp,"-A %s -p tcp %s -m webstr --url \"%s\" -j REJECT --reject-with tcp-reset\n", dtype, timef2, filterstr);
					use_webstr = 1;
				}
			}
	}

	return use_webstr;
}

int
valid_l2w_filter_time()
{
	char *url_time1 = nvram_get("filter_lw_time_x");
	char *url_time2 = nvram_get("filter_lw_time_x_1");
	char starttime1[5], endtime1[5];
	char starttime2[5], endtime2[5];

	memset(starttime1, 0, 5);
	memset(endtime1, 0, 5);
	memset(starttime2, 0, 5);
	memset(endtime2, 0, 5);

	if (!nvram_match("fw_lw_enable_x", "1") && !nvram_match("fw_lw_enable_x_1", "1"))
		return 0;

	if (nvram_match("fw_lw_enable_x", "1"))
	{
		if ((!url_time1) || strlen(url_time1) != 8)
			goto err;

		strncpy(starttime1, url_time1, 4);
		strncpy(endtime1, url_time1 + 4, 4);

		if (atoi(starttime1) >= atoi(endtime1))
			goto err;
	}

	if (nvram_match("fw_lw_enable_x_1", "1"))
	{
		if ((!url_time2) || strlen(url_time2) != 8)
			goto err;

		strncpy(starttime2, url_time2, 4);
		strncpy(endtime2, url_time2 + 4, 4);

		if (atoi(starttime2) >= atoi(endtime2))
			goto err;
	}

	if (nvram_match("fw_lw_enable_x", "1") && nvram_match("fw_lw_enable_x_1", "1"))
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

int
filter_setting(char *wan_if, char *wan_ip, char *lan_if, char *lan_ip, char *logaccept, char *logdrop)
{
	FILE *fp;
	char *proto, *flag, *srcip, *srcport, *dstip, *dstport, *dmz_ip;
	char *setting, *ftype, *dtype;
	char lan_class[32], line[256];
	int i, i_num, i_mac_filter, is_nat_enabled, is_fw_enabled, wport;
	int need_webstr = 0;

	if (!(fp=fopen("/tmp/filter_rules", "w"))) return -1;

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
	i_mac_filter = atoi(nvram_safe_get("macfilter_enable_x"));
	i_num = atoi(nvram_safe_get("macfilter_num_x"));
	if (i_num < 1)
		i_mac_filter = 0;
	if (i_mac_filter > 0)
	{
		dtype = "maclist";
		if (i_mac_filter == 2)
			ftype = logdrop;
		else
			ftype = "RETURN";
		
		g_buf_init();
		for (i=0; i< i_num; i++)
		{
			setting = mac_conv("macfilter_list_x", i, line);
			fprintf(fp, "-A %s -m mac --mac-source %s -j %s\n", dtype, setting, ftype);
		}
		
		if (i_mac_filter != 2)
			fprintf(fp, "-A %s -j %s\n", dtype, logdrop);
	}

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
	fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, logaccept);

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
		/* enable incoming packets from broken dhcp servers, which are sending replies
		 * from addresses other than used for query, this could lead to lower level
		 * of security, but it does not work otherwise (conntrack does not work) :-( 
		 */
		if ( is_physical_wan_dhcp() )
			fprintf(fp, "-A %s -p udp --sport 67 --dport 68 -j %s\n", dtype, logaccept);
		
		// Firewall between WAN and Local
		if (nvram_match("misc_http_x", "1"))
			fprintf(fp, "-A %s -p tcp -d %s --dport %s -j %s\n", dtype, lan_ip, nvram_safe_get("http_lanport"), logaccept);
		
		if (nvram_invmatch("sshd_enable", "0") && nvram_match("sshd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp -d %s --dport 22 -j %s\n", dtype, lan_ip, logaccept);
		
		if (nvram_match("trmd_enable", "1") && is_torrent_support())
		{
			wport=atoi(nvram_safe_get("trmd_pport"));
			if (wport < 1024 || wport > 65535) wport = 51413;
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, wport, logaccept);
			fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, wport, logaccept);
			
			if (nvram_match("trmd_ropen", "1"))
			{
				wport=atoi(nvram_safe_get("trmd_rport"));
				if (wport < 1024 || wport > 65535) wport = 9091;
				fprintf(fp, "-A %s -i %s -p tcp --dport %d -j %s\n", dtype, wan_if, wport, logaccept);
			}
		}
		
		if (nvram_invmatch("enable_ftp", "0") && nvram_match("ftpd_wopen", "1"))
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 21, logaccept);
		
		if (!nvram_match("misc_ping_x", "0"))
		{
			// Pass icmp for ping and udp for traceroute
			fprintf(fp, "-A %s -p icmp -j %s\n", dtype, logaccept);
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
			
			fprintf(fp, "-A %s -i ppp+ -s %s -j %s\n", dtype, lan_class, logaccept);
		}
		
		if (nvram_match("misc_lpr_x", "1"))
		{
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 515, logaccept);
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 9100, logaccept);
			fprintf(fp, "-A %s -p tcp --dport %d -j %s\n", dtype, 3838, logaccept);
		}
		
		// add Virtual Server rules for router host
		if (is_nat_enabled && nvram_match("vts_enable_x", "1"))
		{
			g_buf_init();
			foreach_x("vts_num_x")
			{
				char *proto, *protono, *port, *lport, *dstip;
				char dstports[12];
				
				proto = proto_conv("vts_proto_x", i);
				port = portrange_conv("vts_port_x", i);
				lport = portrange_conv("vts_lport_x", i);
				dstip = ip_conv("vts_ipaddr_x", i);
				
				if ( !strcmp(lan_ip, dstip)  )
				{
					if (lport && strlen(lport)!=0)
						sprintf(dstports, "%s", lport);
					else
						sprintf(dstports, "%s", port);
					
					if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
						fprintf(fp, "-A %s -p tcp -d %s --dport %s -j %s\n", dtype, dstip, dstports, logaccept);
					
					if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
						fprintf(fp, "-A %s -p udp -d %s --dport %s -j %s\n", dtype, dstip, dstports, logaccept);
					
					if (strcmp(proto, "other")==0)
					{
						protono = portrange_conv("vts_protono_x", i);
						fprintf(fp, "-A %s -p %s -d %s -j %s\n", dtype, protono, dstip, logaccept);
					}
				}
			}
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
	// Drop rules always before Accept
	if (valid_l2w_filter_time())
	{
		char lanwan_timematch[128], lanwan_timematch_1[128];
		char ptr[32], *icmplist, *jtype;
		
		timematch_conv(lanwan_timematch, "filter_lw_date_x", "filter_lw_time_x");
		timematch_conv(lanwan_timematch_1, "filter_lw_date_x", "filter_lw_time_x_1");
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
		
		g_buf_init();
		foreach_x("filter_lw_num_x")
		{
			proto = protoflag_conv("filter_lw_proto_x", i, 0);
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_lw_srcip_x", i);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstip = iprange_ex_conv("filter_lw_dstip_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
			setting = filter_conv(proto, flag, srcip, srcport, dstip, dstport); 
			fprintf(fp, "-A %s %s -i %s %s -j %s\n", dtype, lanwan_timematch, lan_if, setting, ftype);
		}
		
		if (nvram_match("fw_lw_enable_x_1", "1"))
		foreach_x("filter_lw_num_x")
		{
			proto = protoflag_conv("filter_lw_proto_x", i, 0);
			flag = protoflag_conv("filter_lw_proto_x", i, 1);
			srcip = iprange_ex_conv("filter_lw_srcip_x", i);
			srcport = portrange_conv("filter_lw_srcport_x", i);
			dstip = iprange_ex_conv("filter_lw_dstip_x", i);
			dstport = portrange_conv("filter_lw_dstport_x", i);
			setting = filter_conv(proto, flag, srcip, srcport, dstip, dstport); 
			fprintf(fp, "-A %s %s -i %s %s -j %s\n", dtype, lanwan_timematch_1, lan_if, setting, ftype);
		}
		
		// ICMP
		foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
			fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", dtype, lanwan_timematch, lan_if, wan_if, ptr, ftype);
		
		if (nvram_match("fw_lw_enable_x_1", "1"))
			foreach(ptr, nvram_safe_get("filter_lw_icmp_x"), icmplist)
				fprintf(fp, "-A %s %s -i %s -o %s -p icmp --icmp-type %s -j %s\n", dtype, lanwan_timematch_1, lan_if, wan_if, ptr, ftype);
		
		// Default
		fprintf(fp, "-A %s -i %s -j %s\n", dtype, lan_if, jtype);
	}

	if (is_fw_enabled)
	{
		if (is_nat_enabled)
		{
			/* Accept to BattleNET */
			if (nvram_match("sp_battle_ips", "1"))
				fprintf(fp, "-A %s -p udp --dport %d -j %s\n", dtype, BATTLENET_PORT, logaccept);
			
			/* Accept to exposed station (DMZ) */
			dmz_ip = nvram_safe_get("dmz_ip");
			if (inet_addr_(dmz_ip))
				fprintf(fp, "-A %s -d %s -j %s\n", dtype, dmz_ip, logaccept);
			
			/* Accept to Virtual Servers */
			if (nvram_match("vts_enable_x", "1"))
			{
				g_buf_init();
				foreach_x("vts_num_x")
				{
					char *proto, *protono, *port, *lport, *dstip;
					char dstips[32], dstports[12];
					
					proto = proto_conv("vts_proto_x", i);
					port = portrange_conv("vts_port_x", i);
					lport = portrange_conv("vts_lport_x", i);
					dstip = ip_conv("vts_ipaddr_x", i);
					
					if ( strcmp(lan_ip, dstip)  )
					{
						if (lport && strlen(lport)!=0)
						{
							sprintf(dstips, "%s:%s", dstip, lport);
							sprintf(dstports, "%s", lport);
						}
						else
						{
							sprintf(dstips, "%s:%s", dstip, port);
							sprintf(dstports, "%s", port);
						}
						
						if (strcmp(proto, "tcp")==0 || strcmp(proto, "both")==0)
							fprintf(fp, "-A %s -p tcp -d %s --dport %s -j %s\n", dtype, dstip, dstports, logaccept);
						
						if (strcmp(proto, "udp")==0 || strcmp(proto, "both")==0)
							fprintf(fp, "-A %s -p udp -d %s --dport %s -j %s\n", dtype, dstip, dstports, logaccept);
						
						if (strcmp(proto, "other")==0)
						{
							protono = portrange_conv("vts_protono_x", i);
							fprintf(fp, "-A %s -p %s -d %s -j %s\n", dtype, protono, dstip, logaccept);
						}
					}
				}
			}
			
			/* Jump to IGD UPnP/NAT-PMP (miniupnpd chain) */
			fprintf(fp, "-A %s -j UPNP\n", dtype);
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
	
	system("iptables-restore /tmp/filter_rules");
	
	if (!need_webstr)
		system("modprobe -r xt_webstr");
	
	return 0;
}

void
default_filter_setting(void)
{
	FILE *fp;
	int is_fw_enabled;

	if (nvram_invmatch("wan_route_x", "IP_Routed")) return;

	if ((fp=fopen("/tmp/filter.default", "w"))==NULL) return;

	fprintf(fp, "*filter\n"
		":INPUT ACCEPT [0:0]\n"
		":FORWARD ACCEPT [0:0]\n"
		":OUTPUT ACCEPT [0:0]\n"
		":UPNP - [0:0]\n"
		":maclist - [0:0]\n"
		":doslimit - [0:0]\n"
		":logaccept - [0:0]\n"
		":logdrop - [0:0]\n");

	is_fw_enabled = nvram_match("fw_enable_x", "1");

	fprintf(fp, "-A INPUT -i lo -j ACCEPT\n");
	fprintf(fp, "-A INPUT -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT\n");
	fprintf(fp, "-A INPUT -i br0 -j ACCEPT\n");

	if (is_fw_enabled)
	{
		fprintf(fp, "-A INPUT -p udp --sport 67 --dport 68 -j ACCEPT\n");
		fprintf(fp, "-A INPUT -j DROP\n");
	}

	fprintf(fp, "-A FORWARD -i br0 -o br0 -j ACCEPT\n");
	fprintf(fp, "-A FORWARD -m state --state INVALID -j DROP\n");
	fprintf(fp, "-A FORWARD -m state --state ESTABLISHED,RELATED -j ACCEPT\n");

	if (is_fw_enabled)
	{
		fprintf(fp, "-A FORWARD -o br0 -j DROP\n");
	}

	fprintf(fp, "-A logaccept -m state --state NEW -j LOG --log-prefix \"ACCEPT \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		    "-A logaccept -j ACCEPT\n");

	fprintf(fp, "-A logdrop -m state --state NEW -j LOG --log-prefix \"DROP \" "
		    "--log-tcp-sequence --log-tcp-options --log-ip-options\n"
		    "-A logdrop -j DROP\n");
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);

	system("iptables-restore /tmp/filter.default");
}


int
start_firewall_ex(char *wan_if, char *wan_ip)
{
	DIR *dir;
	struct dirent *file;
	FILE *fp;
	int i_nf_nat, i_nf_val;
	char tmp[64];
	char name[NAME_MAX];
	char logaccept[32], logdrop[32];
	char *lan_if, *lan_ip, *mcast_ifname;
	char *opt_iptables_script = "/opt/bin/update_iptables.sh";
	char *int_iptables_script = "/etc/storage/post_iptables_script.sh";
	
	if (is_ap_mode())
		return -1;
	
	lan_if = IFNAME_BR;
	lan_ip = nvram_safe_get("lan_ipaddr");
	mcast_ifname = nvram_safe_get("wan0_ifname");
	
	/* mcast needs rp filter to be turned off only for non default iface */
	if (!(nvram_match("mr_enable_x", "1") || nvram_invmatch("udpxy_enable_x", "0")) || (strcmp(wan_if, mcast_ifname) == 0)) 
		mcast_ifname = NULL;
	
	/* Block obviously spoofed IP addresses */
	if (!(dir = opendir("/proc/sys/net/ipv4/conf")))
		perror("/proc/sys/net/ipv4/conf");
	while (dir && (file = readdir(dir))) {
		if (strncmp(file->d_name, ".", NAME_MAX) != 0 && strncmp(file->d_name, "..", NAME_MAX) != 0) 
		{
			sprintf(name, "/proc/sys/net/ipv4/conf/%s/rp_filter", file->d_name);
			if (!(fp = fopen(name, "r+"))) {
				break;
			}
			if (mcast_ifname && strncmp(file->d_name, mcast_ifname, NAME_MAX) == 0)
				fputc('0', fp);
			fclose(fp);
		}
	}
	closedir(dir);
	
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

	if ((fp=fopen("/proc/sys/net/nf_conntrack_nat_mode", "w+")))
	{
		i_nf_val = atoi(nvram_safe_get("nf_nat_type"));
		if (i_nf_val == 2)
			i_nf_nat = 0;	// Linux
		else if (i_nf_val == 1)
			i_nf_nat = 1;	// FCONE
		else
			i_nf_nat = 2;	// RCONE
		sprintf(tmp, "%d", i_nf_nat);
		fputs(tmp, fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/nf_conntrack_max", "w+")))
	{
		i_nf_val = atoi(nvram_safe_get("nf_max_conn"));
		if (i_nf_val < 8192) i_nf_val = 8192;
		if (i_nf_val > 262144) i_nf_val = 262144;
		sprintf(tmp, "%d", i_nf_val);
		fputs(tmp, fp);
		fclose(fp);
		
		if ((fp=fopen("/proc/sys/net/nf_conntrack_max_general", "w+")))
		{
			i_nf_val -= 384;
			sprintf(tmp, "%d", i_nf_val);
			fputs(tmp, fp);
			fclose(fp);
		}
	}

	if ((fp=fopen("/proc/sys/net/nf_conntrack_fastnat", "w+")))
	{
		sprintf(tmp, "%d", is_fastnat_allow());
		fputs(tmp, fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/ip_forward", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	}
	
	/* Tweak NAT performance... */

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_fin_timeout", "w+")))
	{
		fputs("40", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_intvl", "w+")))
	{
		fputs("30", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_probes", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_keepalive_time", "w+")))
	{
		fputs("1800", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_retries2", "w+")))
	{
		fputs("5", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syn_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_synack_retries", "w+")))
	{
		fputs("3", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_recycle", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_tw_reuse", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	/* Tweak DoS-related... */
	if ((fp=fopen("/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_rfc1337", "w+")))
	{
		fputs("1", fp);
		fclose(fp);
	}

	if ((fp=fopen("/proc/sys/net/ipv4/tcp_syncookies", "w+")))
	{
		i_nf_val = (atoi(nvram_safe_get("fw_syn_cook"))) ? 1 : 0;
		sprintf(tmp, "%d", i_nf_val);
		fputs(tmp, fp);
		fclose(fp);
	}
	
	if (check_if_file_exist(int_iptables_script))
	{
		doSystem("%s", int_iptables_script);
	}
	
	if (check_if_file_exist(opt_iptables_script))
	{
		doSystem("%s update", opt_iptables_script);
	}
	
	return 0;
}

