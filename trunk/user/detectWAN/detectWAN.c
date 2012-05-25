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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <unistd.h>
#include <signal.h>

#include <nvram/bcmnvram.h>
#include <semaphore_mfp.h>

#define ETHER_ADDR_STR_LEN	18
#define MAC_BCAST_ADDR		(uint8_t *) "\xff\xff\xff\xff\xff\xff"
#ifdef USE_SINGLE_MAC
#define WAN_IF			"eth2.2"
#else
#define WAN_IF			"eth3"
#endif
#define LAN_IF			"br0"
//#define DEBUG		1

char *get_lan_ipaddr();
char *get_wan_ipaddr();

#include<syslog.h>
#include<stdarg.h>
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

long uptime(void)
{
	struct sysinfo info;

	sysinfo(&info);
	return info.uptime;
}

void
chk_udhcpc()
{
	char *gateway_ip;
	int try_count;

	if (	(nvram_match("wan_route_x", "IP_Routed") && nvram_match("wan0_proto", "dhcp") && !nvram_match("manually_disconnect_wan", "1")) ||
		nvram_match("wan_route_x", "IP_Bridged"))
	{
		if (nvram_match("wan_route_x", "IP_Routed"))
			gateway_ip = nvram_get("wan_gateway_t");
		else
			gateway_ip = nvram_get("lan_gateway_t");

		if (!gateway_ip || (strlen(gateway_ip) < 7) || (!strncmp(gateway_ip, "0.0.0.0", 7)))
		{
			if (nvram_match("dw_debug", "1"))
				fprintf(stderr, "[detectWAN] invalid gateway ip\n");

			return;
		}

		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			if (!strcmp(get_wan_ipaddr(), "0.0.0.0"))
			{
				if (nvram_match("dw_debug", "1"))
					fprintf(stderr, "[detectWAN] invalid gateway ip\n");

				return;
			}
		}
		else
		{
			if (!strcmp(get_lan_ipaddr(), "0.0.0.0"))
			{
				if (nvram_match("dw_debug", "1"))
					fprintf(stderr, "[detectWAN] invalid gateway ip\n");

				return;
			}
		}

		if (nvram_match("dhcp_renew", "1"))
		{
			if (nvram_match("dw_debug", "1"))
				fprintf(stderr, "[detectWAN] skip udhcpc refresh...\n");

			return;
		}
		else
		{
			nvram_set("dhcp_renew", "1");
		}

		if (nvram_match("dw_debug", "1"))
			fprintf(stderr, "[detectWAN] try to refresh udhcpc\n");

		if (nvram_match("wan_route_x", "IP_Routed"))
		{
			{
#if 0
				system("/sbin/stop_wanduck");
#endif

				logmessage("detectWAN", "perform DHCP renew");
				system("killall -SIGUSR1 udhcpc");

#if 0
				try_count = 0;
				while (pids("wanduck") && (++try_count < 10))
					sleep(1);

				try_count = 0;
				while (!pids("wanduck") && (++try_count < 10))
				{
					sleep(3);
					system("/sbin/start_wanduck");
				}
#endif
			}
		}
		else
		{
			logmessage("detectWAN", "perform DHCP renew");
			system("killall -SIGUSR1 udhcpc");
		}
	}
}

char *
get_lan_ipaddr()
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *inaddr;
	struct in_addr ip_addr;

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return strdup("0.0.0.0");

	strncpy(ifr.ifr_name, LAN_IF, IFNAMSIZ);
	inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	inet_aton("0.0.0.0", &inaddr->sin_addr);	

	/* Get IP address */
	ioctl(s, SIOCGIFADDR, &ifr);
	close(s);	

	ip_addr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;
#ifdef DEBUG
	fprintf(stderr, "current LAN IP address: %s\n", inet_ntoa(ip_addr));
#endif
	return inet_ntoa(ip_addr);
}

char *
get_wan_ipaddr()
{
	int s;
	struct ifreq ifr;
	struct sockaddr_in *inaddr;
	struct in_addr ip_addr;

	if (nvram_match("wan_route_x", "IP_Bridged"))
		return strdup("0.0.0.0");

	/* Retrieve IP info */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return strdup("0.0.0.0");

	if (nvram_match("wan0_proto", "dhcp") || nvram_match("wan0_proto", "static"))
		strncpy(ifr.ifr_name, WAN_IF, IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ);
	inaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	inet_aton("0.0.0.0", &inaddr->sin_addr);	

	/* Get IP address */
	ioctl(s, SIOCGIFADDR, &ifr);
	close(s);	

	ip_addr = ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr;
#ifdef DEBUG
	fprintf(stderr, "current WAN IP address: %s\n", inet_ntoa(ip_addr));
#endif
	return inet_ntoa(ip_addr);
}

struct arpMsg {
	/* Ethernet header */
	uint8_t  h_dest[6];			/* destination ether addr */
	uint8_t  h_source[6];			/* source ether addr */
	uint16_t h_proto;			/* packet type ID field */

	/* ARP packet */
	uint16_t htype;				/* hardware type (must be ARPHRD_ETHER) */
	uint16_t ptype;				/* protocol type (must be ETH_P_IP) */
	uint8_t  hlen;				/* hardware address length (must be 6) */
	uint8_t  plen;				/* protocol address length (must be 4) */
	uint16_t operation;			/* ARP opcode */
	uint8_t  sHaddr[6];			/* sender's hardware address */
	uint8_t  sInaddr[4];			/* sender's IP address */
	uint8_t  tHaddr[6];			/* target's hardware address */
	uint8_t  tInaddr[4];			/* target's IP address */
	uint8_t  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
} ATTRIBUTE_PACKED;

/* args:	yiaddr - what IP to ping
 *		ip - our ip
 *		mac - our arp address
 *		interface - interface to use
 * retn:	1 addr free
 *		0 addr used
 *		-1 error
 */
                                                                                                              
static const int one = 1;

int setsockopt_broadcast(int fd)
{
    return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
}

/* FIXME: match response against chaddr */
int arpping(/*uint32_t yiaddr, uint32_t ip, uint8_t *mac, char *interface*/)
{
	uint32_t yiaddr;
	uint32_t ip;
	uint8_t mac[6];
	char wanmac[18];
	char tmp[3];
	int i, ret;
	char DEV[8];
	char *gateway_ip;

	if (nvram_match("wan_route_x", "IP_Routed"))
		gateway_ip = nvram_get("wan_gateway_t");
	else
		gateway_ip = nvram_get("lan_gateway_t");

	if (!gateway_ip || (strlen(gateway_ip) < 7) || (!strncmp(gateway_ip, "0.0.0.0", 7)))
	{
		return 1;
	}

	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		inet_aton(nvram_safe_get("wan_gateway_t"), &yiaddr);
		inet_aton(get_wan_ipaddr(), &ip);
		strcpy(wanmac, nvram_safe_get("wan0_hwaddr"));	// WAN MAC address
	}
	else
	{
		inet_aton(nvram_safe_get("lan_gateway_t"), &yiaddr);
		inet_aton(get_lan_ipaddr(), &ip);
		strcpy(wanmac, nvram_safe_get("lan_hwaddr"));	// br0 MAC address
	}

        wanmac[17]=0;
        for(i=0;i<6;i++)
        {
                tmp[2]=0;
                strncpy(tmp, wanmac+i*3, 2);
                mac[i]=strtol(tmp, (char **)NULL, 16);
        }

	int	timeout = 2;
	int	s;			/* socket */
	int	rv = 0;			/* return value */
	struct sockaddr addr;		/* for interface name */
	struct arpMsg	arp;
	fd_set		fdset;
	struct timeval	tm;
	time_t		prevTime;


	s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if (s == -1) {
#ifdef DEBUG
		fprintf(stderr, "cannot create raw socket\n");
#endif
		return 0;
	}

	if (setsockopt_broadcast(s) == -1) {
#ifdef DEBUG		
		fprintf(stderr, "cannot setsocketopt on raw socket\n");
#endif
		close(s);
		return 0;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.h_dest, MAC_BCAST_ADDR, 6);		/* MAC DA */
	memcpy(arp.h_source, mac, 6);			/* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);			/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &yiaddr, sizeof(yiaddr));	/* target IP address */

	memset(&addr, 0, sizeof(addr));
	memset(DEV, 0, sizeof(DEV));
	if (nvram_match("wan_route_x", "IP_Routed"))
		strcpy(DEV, WAN_IF);
	else
		strcpy(DEV, LAN_IF);
	strncpy(addr.sa_data, DEV, sizeof(addr.sa_data));

	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, DEV, IFNAMSIZ) != 0)	// J++
        {
#ifdef DEBUG
                fprintf(stderr, "setsockopt error: %s\n", DEV);
                perror("setsockopt set:");
#endif
        }

	ret = sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr));

        if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, "", IFNAMSIZ) != 0)	// J++
        {
#ifdef DEBUG
                fprintf(stderr, "setsockopt error: %s\n", "");
                perror("setsockopt reset:");
#endif
        }

	if (ret < 0)
	{
		sleep(1);
		return 0;
	}

	/* wait arp reply, and check it */
	tm.tv_usec = 0;
	prevTime = uptime();
	while (timeout > 0) {
		FD_ZERO(&fdset);
		FD_SET(s, &fdset);
		tm.tv_sec = timeout;
		if (select(s + 1, &fdset, (fd_set *) NULL, (fd_set *) NULL, &tm) < 0) {
#ifdef DEBUG
			fprintf(stderr, "error on ARPING request\n");
#endif
			if (errno != EINTR) rv = 0;
		} else if (FD_ISSET(s, &fdset)) {
			if (recv(s, &arp, sizeof(arp), 0) < 0 ) rv = 0;
			if (arp.operation == htons(ARPOP_REPLY) &&
			    memcmp(arp.tHaddr, mac, 6) == 0 &&
			    *((uint32_t *) arp.sInaddr) == yiaddr) {
#ifdef DEBUG
				fprintf(stderr, "Valid arp reply from [%02X:%02X:%02X:%02X:%02X:%02X]\n",
					(unsigned char)arp.sHaddr[0],
					(unsigned char)arp.sHaddr[1],
					(unsigned char)arp.sHaddr[2],
					(unsigned char)arp.sHaddr[3],
					(unsigned char)arp.sHaddr[4],
					(unsigned char)arp.sHaddr[5]);
#endif
				close(s);
				rv = 1;
				return 1;
			}
		}
		timeout -= uptime() - prevTime;
		prevTime = uptime();
	}

	close(s);
#ifdef DEBUG
	fprintf(stderr, "%salid arp reply\n", rv ? "V" : "No v");
#endif
	return rv;
}

int is_phyconnected()
{
	if (nvram_match("wan_route_x", "IP_Routed"))
	{
		if (nvram_match("link_wan", "1"))
			return 1;
		else
			return 0;
	}
	else
	{
		if (nvram_match("link_lan", "1"))
			return 1;
		else
			return 0;
	}
}

#define MAX_ARP_RETRY 3

int detectWAN_arp()
{
	int count;

	while (1)
	{
		count = 0;

		while (count < MAX_ARP_RETRY)
		{
			if (nvram_match("wan_route_x", "IP_Routed") && !is_phyconnected())
			{
				if (nvram_match("dw_debug", "1"))
					fprintf(stderr, "[detectWAN] phy disconnected\n");

				count++;
				sleep(2);
			}
			else if (arpping())
			{
				if (nvram_match("dw_debug", "1"))
					fprintf(stderr, "[detectWAN] got response from gateway\n");

				break;
			}
			else
			{
				if (nvram_match("dw_debug", "1"))
					fprintf(stderr, "[detectWAN] no response from gateway\n");

				count++;
			}

			if (nvram_match("dw_debug", "1"))
				fprintf(stderr, "[detectWAN] count: %d\n", count);
		}

		if (is_phyconnected() && (count >= MAX_ARP_RETRY))
		{
			chk_udhcpc();
		}

		sleep(20);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int ret;
	char *gateway_ip;

	/* write pid */
	if ((fp = fopen("/var/run/detectWan.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	for(;;)
	{
		if (nvram_match("wan_route_x", "IP_Routed"))
			gateway_ip = nvram_get("wan_gateway_t");
		else
			gateway_ip = nvram_get("lan_gateway_t");

		/* if not valid gateway, poll for it at first */
		if (!gateway_ip || (strlen(gateway_ip) < 7) || (!strncmp(gateway_ip, "0.0.0.0", 7)))
		{
			if (nvram_match("dw_debug", "1"))
				fprintf(stderr, "[detectWAN] no valid gateway\n");

			sleep(15);
		}
		/* valid gateway for now */
		else
		{
			if (nvram_match("dw_debug", "1"))
				fprintf(stderr, "[detectWAN] got valid gateway\n");

			break;
		}
	}

	ret = detectWAN_arp();

	if (ret < 0)
		printf("Failure!\n");
	else
		printf("Success!\n");
	
	return 0;
}
