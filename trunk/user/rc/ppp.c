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
 * ppp scripts
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>

#include "rc.h"
#include <semaphore_mfp.h>

/*
* parse ifname to retrieve unit #
*/
int
ppp_ifunit(char *ifname)
{
	if (strncmp(ifname, "ppp", 3))
		return -1;
	if (!isdigit(ifname[3]))
		return -1;
	return atoi(&ifname[3]);
}

/*
 * Called when VPN client link comes up
 */
int
ipup_main(int argc, char **argv)
{
	FILE *fp;
	char *wan_ifname = safe_getenv("IFNAME");
	char *value;
	char buf[256];
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";
	int fd;
	struct ifreq ifr;

	umask(0000);
 
	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;
	
	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	/* Touch connection file */
	if (!(fp = fopen(strcat_r("/tmp/ppp/link.", wan_ifname, tmp), "a"))) {
		perror(tmp);
		return errno;
	}
	fclose(fp);

	if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
		return -1;

	if ((value = getenv("IPLOCAL"))) {
		ifconfig(wan_ifname, IFUP,
			 value, "255.255.255.255");
		nvram_set(strcat_r(prefix, "ipaddr", tmp), value);
		nvram_set(strcat_r(prefix, "netmask", tmp), "255.255.255.255");
	}

	if ((value = getenv("IPREMOTE")))
		nvram_set(strcat_r(prefix, "gateway", tmp), value);
	strcpy(buf, "");
	if (getenv("DNS1"))
		sprintf(buf, "%s", getenv("DNS1"));
	if (getenv("DNS2"))
		sprintf(buf + strlen(buf), "%s%s", strlen(buf) ? " " : "", getenv("DNS2"));
	nvram_set(strcat_r(prefix, "dns", tmp), buf);

/* Isn't it set by pppd + rp-pppoe plugin?*/
	if (nvram_match("wan0_proto", "pppoe"))
	{
		if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		{
        		/* Give a more helpful message for the common error case */
			if (errno == EPERM)
				perror("Cannot create raw socket");

			perror("socket");
		}	
		else
		{	
			strncpy(ifr.ifr_name, wan_ifname, sizeof(ifr.ifr_name));

			if (ioctl(fd, SIOCGIFMTU, &ifr) < 0)
				perror("ioctl(SIOCGIFMTU)");
			else
			{	
				dbg("current MTU: %d\n", ifr.ifr_mtu);
				if (	ifr.ifr_mtu != atoi(nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp))) &&
					atoi(nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp))) <= 1492	)
				{
					dbg("change MTU manually...\n");
					doSystem("ifconfig ppp%d mtu %s", unit, nvram_safe_get(strcat_r(prefix, "pppoe_mtu", tmp)));
				}
			}	

			close(fd);
		}
	}

	wan_up(wan_ifname);

	logmessage(nvram_safe_get("wan_proto_t"), "connect to ISP");
	wanmessage("");

	return 0;
}

/*
 * Called when VPN client link goes down
 */
int
ipdown_main(int argc, char **argv)
{
	char *wan_ifname = safe_getenv("IFNAME");
	int unit;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	umask(0000);

	if ((unit = ppp_ifunit(wan_ifname)) < 0)
		return -1;

	snprintf(prefix, sizeof(prefix), "wan%d_", unit);

	if (!nvram_get(strcat_r(prefix, "ifname", tmp)))
		return -1;

//	ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
//	logmessage(nvram_safe_get("wan_proto_t"), "ifconfig %s 0.0.0.0", wan_ifname);

	wan_down(wan_ifname);

	unlink(strcat_r("/tmp/ppp/link.", wan_ifname, tmp));

	preset_wan_routes(wan_ifname);

	logmessage(nvram_safe_get("wan_proto_t"), "Disconnected");

	wanmessage(pppstatus(tmp));

	return 0;
}


int
ipup_vpns_main(int argc, char **argv)
{
	FILE *fp;
	int i_cast;
	char *peer_name;

	if (argc < 7)
		return -1;

	peer_name = safe_getenv("PEERNAME");

	logmessage("vpn server", "peer connected - ifname: %s, local IP: %s, remote IP: %s, login: %s",
			argv[1], argv[5], argv[6], peer_name);

	spinlock_lock(SPINLOCK_VPNSCli);
	fp = fopen("/tmp/vpns.leases", "a+");
	if (fp)
	{
		fprintf(fp, "%s %s %s %s\n", argv[1], argv[5], argv[6], peer_name);
		fclose(fp);
	}
	spinlock_unlock(SPINLOCK_VPNSCli);

	if (!pids("bcrelay"))
	{
		i_cast = atoi(nvram_safe_get("vpns_cast"));
		if (i_cast == 1 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", IFNAME_BR, "-o", "ppp[1-2][0-9]", "-n");
		if (i_cast == 2 || i_cast == 3)
			eval("/usr/sbin/bcrelay", "-d", "-i", "ppp[1-2][0-9]", "-o", IFNAME_BR, "-n");
	}

	return 0;
}

int
ipdown_vpns_main(int argc, char **argv)
{
	FILE *fp1, *fp2;
	int i_clients;
	char ifname[16], addr_l[32], addr_r[32], peer_name[64];
	char* clients_l1 = "/tmp/vpns.leases";
	char* clients_l2 = "/tmp/.vpns.leases";
	char* svcs[] = { "bcrelay", NULL };

	if (argc < 2)
		return -1;

	logmessage("vpn server", "peer disconnected - ifname: %s", argv[1]);

	i_clients = 0;
	spinlock_lock(SPINLOCK_VPNSCli);
	fp1 = fopen(clients_l1, "r");
	fp2 = fopen(clients_l2, "w");
	if (fp1)
	{
		while(fscanf(fp1, "%s %s %s %[^\n]\n", ifname, addr_l, addr_r, peer_name) == 4)
		{
			if (strcmp(ifname, argv[1]))
			{
				i_clients++;
				if (fp2)
					fprintf(fp2, "%s %s %s %s\n", ifname, addr_l, addr_r, peer_name);
			}
		}
		
		fclose(fp1);
	}
	if (fp2)
	{
		fclose(fp2);
		rename(clients_l2, clients_l1);
		unlink(clients_l2);
	}
	spinlock_unlock(SPINLOCK_VPNSCli);

	if (i_clients == 0 && pids(svcs[0]))
		kill_services(svcs, 3, 1);

	return 0;
}

