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
#ifdef QOS
#include<stdio.h>
#include<stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <dirent.h>

#ifdef ASUS_RC
#include<asusbcmnvram.h>
#else
#include <epivers.h>
#include<nvram/bcmnvram.h>
#endif


#include <shutils.h>
FILE *fp=NULL;

/***************************************************************************************/
//  The definitions of all partations
//    10:1 root class for upload, ie: wan if
//    10:2 root class for download, ie: lan if
//    10:10 partation for tos(bit 4)	& short pkt 	& psh
//    10:20 partation for tos(bit 3)	& UD(high) 
//    10:30 partation for tos(bit 2)	& application	
//    10:40 partation for ftp on router& UD(middle)
//    10:50 partation for tos(bit 1)
//    10:60 default class
//    10:70 partation for UD(low)
//    10:80 partation only for vlan
//    10:100 added a case for lan access with unlimited bandwidth
/***************************************************************************************/

void tc_class_global(char *network, char *idx, char *minBW, char *maxBW, char *prio_level)
{
	char flowid[32];
	char filter_handle[32];
	char handle[32];

	sprintf(flowid, "%s%s", "10:", idx);
	sprintf(handle, "%s%s%s", "1", idx, ":");
	sprintf(filter_handle, "%s", idx);

	eval("tc","class","add","dev",network,"parent","10:1","classid",flowid,"htb","rate",\
		minBW,"ceil",maxBW, "prio", prio_level);

	eval("tc","qdisc","add","dev",network,"parent",flowid,"handle",handle,"pfifo");

	eval("tc","filter","add","dev",network,"parent","10:", "protocol", "ip", "prio", \
		prio_level,"handle", filter_handle, "fw", "classid", flowid);

#ifdef QOS_DEBUG
	syslog(LOG_NOTICE, "tc class add dev %s parent 10:1 classid %s htb rate %s ceil %s prio %s\n",\
		network, flowid, minBW, maxBW, prio_level);
	syslog(LOG_NOTICE, "tc qdisc add dev %s parent %s handle %s pfifo\n", network, flowid, handle);
	syslog(LOG_NOTICE, "tc filter add dev %s parent 10: protocol ip prio %s handle %s fw classid %s\n",\
		network, prio_level, filter_handle, flowid);
	printf("tc class add dev %s parent 10:1 classid %s htb rate %s ceil %s prio %s\n", network,\
		flowid, minBW, maxBW, prio_level);
	printf("tc qdisc add dev %s parent %s handle %s pfifo\n", network, flowid, handle);
	printf("tc filter add dev %s parent 10: protocol ip prio %s handle %s fw classid %s\n", network,\
		prio_level, filter_handle, flowid);
#endif
}

/************************************************************************************/
/* This function is to add the filter match user specify. For LAN IP address, the   */
/* interface should be br0. For WAN IP address, the interface should be eth3/ppp0   */
/************************************************************************************/
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

void tc_filter(char *ipaddr, char *port, int prio)
{	
	int user_mark;
	user_mark= prio*10;
		
	if (port == NULL)
	{
		fprintf(fp, "-A PREROUTING -s %s -j MARK --set-mark %d\n", ipaddr,user_mark);
	}
	else if (ipaddr == NULL)
	{
		fprintf(fp, "-A PREROUTING -p tcp --dport %s -j MARK --set-mark %d\n", port, user_mark);
		fprintf(fp, "-A PREROUTING -p udp --dport %s -j MARK --set-mark %d\n", port, user_mark);
	}
	else
	{
		fprintf(fp, "-A PREROUTING -s %s -p tcp --dport %s -j MARK --set-mark %d\n", ipaddr, port, user_mark);
		fprintf(fp, "-A PREROUTING -s %s -p udp --dport %s -j MARK --set-mark %d\n",ipaddr, port, user_mark);
	}
}

/**
 * GAME: qos_pshack_prio, INTERNET: qos_shortpkt_prio, FTP: qos_service_enable, VOIP(limited P2P): qos_tos_prio
 */
void start_qos(char *wan_ipaddr)	//modified by angela 2008.05
{
	int rulenum=0;
	int idx_class=0, idx_filter=0, idx_class_S=0; // control index
	int idx=0;
	char wan_net_name[32], lan_net_name[32];
	char qos_name[32];
	int ubw=0, service_ubw=0, m_ubw=0;
	char qos_ubw[32],qos_min_ubw[32], qos_data_ubw[32], qos_down[32], qos_default_ubw[32];
	char dfragment_size[32];
	int flag=0;
	int avg_rule_ubw = 0, hi_ubw=0, mid_ubw=0, low_ubw=0;
	char qos_hi_ubw[32], qos_mid_ubw[32], qos_low_ubw[32];
#undef GPQOS

//	nvram_set("qos_XXX", "1");
	rulenum=atoi(nvram_safe_get("qos_rulenum_x"));
	
	//Open file for writing iptables rules
	if ((fp=fopen("/tmp/mangle_rules", "w")) == NULL) return;
	fprintf(fp, "*mangle\n");
	
	/*Enable Dynamic Fragmentation function*/
	/* 536 =  standard MTU(576) - IP header(20) - TCP header(20) */
	/* 1200 = suggested trasmmit packet size */
	if ( nvram_match("qos_dfragment_enable", "1")) {
		int mss_size = 1200;
		mss_size = 536 + (1200-536)*atoi(nvram_safe_get("qos_dfragment_size"))/100;

		fprintf(fp, "-A POSTROUTING -p tcp --tcp-flags SYN,ACK SYN,ACK -j TCPMSS --set-mss %d\n", mss_size);
	}

	/* Get interface name */
	if (nvram_match("wan0_proto", "pppoe") || nvram_match("wan0_proto","pptp") || nvram_match("wan0_proto","l2tp"))
		strcpy(wan_net_name, nvram_safe_get("wan0_pppoe_ifname"));
	else	
		strcpy(wan_net_name, nvram_safe_get("wan0_ifname"));

	strcpy(lan_net_name, nvram_safe_get("lan_ifname"));

	dbg("QoS wan_net_name: %s, lan_net_name: %s\n", wan_net_name, lan_net_name);

	/* Add new qdisc for wan classify */
	eval("tc","qdisc","add", "dev", wan_net_name, "root", "handle", "10:",\
		 "htb", "default", "51");

	/* Add root class 10:2(for download) and BW = unlimited */

        //eval("tc", "class", "add", "dev", lan_net_name, "parent", "10:", "classid", "10:2",\
                 "htb", "rate", "1000000kbit", "ceil", "1000000kbit");

	// tmp declare here
	char lan_ipaddr[32];
	sprintf(lan_ipaddr, "%s%s", nvram_safe_get("lan_ipaddr"), "/24");

	/*Decide all the max/min=qos_ubw/qos_ubw bandwidth for each classes */		
	ubw = atoi(nvram_safe_get("qos_ubw_real"));
	dbg("upload speed to apply: %s\n", nvram_safe_get("qos_ubw_real"));
	m_ubw = ubw/5;
	if (m_ubw == 0)  m_ubw = 1;
	if (rulenum > 0)
	{
		avg_rule_ubw = ubw/rulenum;
		hi_ubw = avg_rule_ubw*0.85;
		mid_ubw = avg_rule_ubw*0.51;
		low_ubw = avg_rule_ubw*0.17;

		if (hi_ubw <= 0)	hi_ubw= 20;
		if (mid_ubw <= 0)	mid_ubw= 10;
		if (low_ubw <= 0)	low_ubw= 1;
		printf("[ubw chk] avg:%d, h:%d, m:%d, l:%d\n", avg_rule_ubw, hi_ubw, mid_ubw, low_ubw);	// tmp test
	}
	
	memset(qos_ubw, 0, sizeof(qos_ubw));
	memset(qos_min_ubw, 0, sizeof(qos_min_ubw));
	memset(qos_default_ubw, 0, sizeof(qos_default_ubw));
	sprintf(qos_ubw, "%d%s", ubw, "kbit");
	sprintf(qos_min_ubw,"1bps");
	sprintf(qos_default_ubw, "%d%s", m_ubw, "kbit");
	if (rulenum > 0)
	{
		memset(qos_hi_ubw, 0, sizeof(qos_hi_ubw));
		memset(qos_mid_ubw, 0, sizeof(qos_mid_ubw));
		memset(qos_low_ubw, 0, sizeof(qos_low_ubw));
		sprintf(qos_hi_ubw, "%d%s", hi_ubw, "kbit");
		sprintf(qos_mid_ubw, "%d%s", mid_ubw, "kbit");
		sprintf(qos_low_ubw, "%d%s", low_ubw, "kbit");
	}

	/* --Add class for global settings / user specify / server services-- */
	/* Add root class 10:1(for upload) and BW = qos_ubw */
	eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:", "classid", "10:1",\
		 "htb", "rate", qos_ubw, "ceil", qos_ubw);		

	// Add class for GAME
	if (nvram_match("qos_pshack_prio", "1")) {
		tc_class_global(wan_net_name, "10", qos_min_ubw, qos_ubw, "2");
		flag |= 0x01;
	}

	// Add class for Application like Internet
	if (nvram_match("qos_shortpkt_prio", "1")) {
		if ((flag & 0x10) == 0) {
			tc_class_global(wan_net_name, "20", qos_min_ubw, qos_ubw, "3");
			flag |= 0x10;
		}

		//Add iptables rule of Applications
		if ((flag & 0x40) == 0) {
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 80 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 80 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 8080 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 8080 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 443 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 443 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 53 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 53 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p udp -m udp --dport 53 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p udp -m udp --dport 53 -j RETURN\n");
			flag |= 0x40;
		}
	}


	// Add class for services such as ftp on router 10:30 priority 4
	if (nvram_match("qos_service_enable", "1")) {
		tc_class_global(wan_net_name, "30", qos_min_ubw, qos_ubw, "4");

		//Add iptables rule of ftp
		fprintf(fp, "-A POSTROUTING -m helper --helper ftp -j MARK --set-mark 30\n");
		fprintf(fp, "-A POSTROUTING -m helper --helper ftp -j RETURN\n");
	}

	// Add class for VOIP, p2p must be limited here
	if (nvram_match("qos_tos_prio", "1")) {
		if ((flag & 0x01) == 0) {
			tc_class_global(wan_net_name, "10", qos_min_ubw, qos_ubw, "2");
			flag |= 0x01;
		}

		if ((flag & 0x10) == 0) {
			tc_class_global(wan_net_name, "20", qos_min_ubw, qos_ubw, "3");
			flag |= 0x10;
		}

		if ((flag & 0x40) == 0) {
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 80 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 80 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 8080 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 8080 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 443 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 443 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 53 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p tcp -m tcp --dport 53 -j RETURN\n");
			fprintf(fp, "-A POSTROUTING -p udp -m udp --dport 53 -j MARK --set-mark 20\n");
			fprintf(fp, "-A POSTROUTING -p udp -m udp --dport 53 -j RETURN\n");
			flag |= 0x40;
		}

		/* Add root class 10:2(for download) and BW = unlimited */
		if (atoi(nvram_safe_get("qos_ubw_real")) >= 640)
			sprintf(qos_down,"1Mbit");
		else
			sprintf(qos_down,"256kbit");

		eval("tc","qdisc","add", "dev", wan_net_name, "root", "handle", "10:", "htb", "default", "91");

        	eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:", "classid", "10:2",\
                	 "htb", "rate", "1000000kbit", "ceil", "1000000kbit");

                eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:2", "classid", "10:70",\
                        "htb", "rate", "950000kbit", "ceil", "950000kbit", "prio", "1" );
                eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:70", "handle", "170:", "pfifo");
                eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", "prio", "1", \
                        "handle", "70", "fw", "classid", "10:70");

		eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:2", "classid", "10:80",\
			"htb", "rate", "950000kbit", "ceil", "950000kbit", "prio", "8" );
		eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:80", "handle", "180:", "pfifo");
		eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", "prio", "8", \
			"handle", "80", "fw", "classid", "10:80");

		eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:2", "classid", "10:90",\
			"htb", "rate", qos_down, "ceil", qos_down, "prio", "9" );
		eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:90", "handle", "190:", "pfifo");
		eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", "prio", "9", \
			"handle", "90", "fw", "classid", "10:90");

		eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:2", "classid", "10:91",\
			"htb", "rate", qos_down, "ceil", qos_down, "prio", "9" );
		eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:91", "handle", "191:", "pfifo");
		eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", "prio", "9", \
			"handle", "91", "fw", "classid", "10:91");

		/* Add the class 10:100 for LAN access the router */
		eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:2", "classid", "10:100",\
			"htb", "rate", "950000kbit", "ceil", "950000kbit", "prio", "8" );
		eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:100", "handle", "180:", "pfifo");
		eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", "prio", "8", \
			"handle", "100", "fw", "classid", "10:100");

		fprintf(fp, "-A POSTROUTING -s %s -d %s -j MARK --set-mark 100\n", nvram_safe_get("lan_ipaddr"), lan_ipaddr);
		fprintf(fp, "-A POSTROUTING -s %s -d %s -j RETURN\n", nvram_safe_get("lan_ipaddr"), lan_ipaddr);
	}

	/* Add class for User specify, 10:20(high), 10:40(middle), 10:60(low)*/
	if (atoi(nvram_safe_get("qos_rulenum_x"))) {
		for (idx_class=0; idx_class < rulenum; idx_class++) {
			/*tc_class(wan_net_name, num_conv(idx_class),\
				 qos_userspec_ubw, qos_other_max, Ch_conv("qos_prio_x", idx_class));*/
			switch(atoi(Ch_conv("qos_prio_x", idx_class))) {
				case 1:
					if ((flag & 0x01) == 0) {						
						//tc_class_global(wan_net_name, "10", qos_min_ubw, qos_ubw, "2");
						tc_class_global(wan_net_name, "10", qos_hi_ubw, qos_ubw, "2");
						flag |= 0x01;
					}
					break;
				case 4:
					if ((flag & 0x02) == 0) {
						//tc_class_global(wan_net_name, "40", qos_min_ubw, qos_ubw, "5");
						tc_class_global(wan_net_name, "40", qos_mid_ubw, qos_ubw, "5");
						flag |= 0x02;
					}
					break;
				case 6:
					if ((flag & 0x04) == 0) {
						//tc_class_global(wan_net_name, "60", qos_min_ubw, qos_ubw, "7");
						tc_class_global(wan_net_name, "60", qos_low_ubw, qos_ubw, "7");
						flag |= 0x04;
					}
					break;
			}

			tc_filter(Ch_conv("qos_ip_x", idx_class), \
	      			Ch_conv("qos_port_x", idx_class), atoi(Ch_conv("qos_prio_x", idx_class)));

		}
	}
		
	/*Add the default class 10:10 */		
	tc_class_global(wan_net_name, "50", qos_min_ubw, qos_ubw, "6");
	//TFA parent ffff: protocol ip handle 50 fw police rate 100kbit mtu 12k burst 10k drop
					
	eval("tc", "class", "add", "dev", wan_net_name, "parent", "10:1", "classid", "10:51",\
		 "htb", "rate", qos_default_ubw, "ceil", qos_ubw, "prio", "6");	// 0723 ASUS
		 //"htb", "rate", qos_min_ubw, "ceil", qos_ubw, "prio", "6");

	eval("tc", "qdisc", "add", "dev", wan_net_name, "parent", "10:51", "handle", "151:", "pfifo");

	//eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip", \
	// 	"handle", "51", "fw", "police", "rate", "10kbit", "mtu", "10k", "burst", "1k", "drop");
	eval("tc", "filter", "add", "dev", wan_net_name, "parent", "10:", "protocol", "ip",\
	"prio", "6" ,"handle", "51", "fw", "classid", "10:51");	// 0723 ASUS

	/*Enable Dynamic Fragmentation function*/
	/* 536 =  standard MTU(576) - IP header(20) - TCP header(20) */
	/* 1200 = suggested trasmmit packet size */

	if ( nvram_match("qos_dfragment_enable", "1")) {

		//fprintf(fp, "-A POSTROUTING -p tcp --tcp-flags SYN,ACK SYN,ACK \
		//		-j TCPMSS --set-mss %s\n",nvram_safe_get("qos_dfragment_size"));

		int mss_size = 1200;
		mss_size = 536 + (1200-536)*atoi(nvram_safe_get("qos_dfragment_size"))/100;

		fprintf(fp, "-A POSTROUTING -p tcp --tcp-flags SYN,ACK SYN,ACK -j TCPMSS --set-mss %d\n", mss_size);
	}
		
	/*write "COMMIT" to /tmp/mangle_rules to end after writing all rules*/
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
		
	/*recover all iptables rules*/
	system("iptables-restore /tmp/mangle_rules");
}

void start_dfragment_standalone()
{
	FILE *fp;
	
	//Open file for writing iptables rules
	if ((fp=fopen("/tmp/mangle_rules", "w")) == NULL) return;
	fprintf(fp, "*mangle\n");
	
	/*Enable Dynamic Fragmentation function*/
	/* 536 =  standard MTU(576) - IP header(20) - TCP header(20) */
	/* 1200 = suggested trasmmit packet size */
	if ( nvram_match("qos_dfragment_enable", "1"))
	{
		int mss_size = 1200;
		mss_size = 536 + (1200-536)*atoi(nvram_safe_get("qos_dfragment_size"))/100;

		fprintf(fp, "-A POSTROUTING -p tcp --tcp-flags SYN,ACK SYN,ACK -j TCPMSS --set-mss %d\n", mss_size);
	}
	else
	{
	        /* Clean iptables*/
        	fprintf(fp, "-F\n");
	}
		
	/*write "COMMIT" to /tmp/mangle_rules to end after writing all rules*/
	fprintf(fp, "COMMIT\n\n");
	fclose(fp);
		
	/*recover all iptables rules*/
	system("iptables-restore /tmp/mangle_rules");
}
#endif
