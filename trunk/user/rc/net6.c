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
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <dirent.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"

#if defined (USE_IPV6)

#if defined (HAVE_GETIFADDRS)
#include <ifaddrs.h>
#endif

void init_ipv6(void)
{
	int ipv6_type = get_ipv6_type();
	control_if_ipv6_all((ipv6_type == IPV6_DISABLED) ? 0 : 1);
	reset_lan6_vars();
}

void control_if_ipv6_all(int enable)
{
	int i;
	char tmp[64];
	char* if6_off[] = { "default", "all", NULL };
	char* rad_off[] = { "default", "lo", "sit0", IFNAME_MAC, NULL };
#if defined (USE_SINGLE_MAC)
	char* if6_on[] = { "default", "lo", IFNAME_MAC, IFNAME_BR, IFNAME_WAN, NULL };
#else
	char* if6_on[] = { "default", "lo", IFNAME_BR, IFNAME_WAN, NULL };
#endif
	
	if (!enable)
	{
#if defined (USE_KERNEL3X)
		for (i=0; if6_off[i] != NULL; i++)
		{
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", if6_off[i]);
			fput_int(tmp, 1);
		}
#endif
		for (i=0; rad_off[i] != NULL; i++)
		{
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/accept_ra", rad_off[i]);
			fput_int(tmp, 0);
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/accept_ra_pinfo", rad_off[i]);
			fput_int(tmp, 0);
		}
		
		sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/forwarding", "all");
		fput_int(tmp, 0);
	}
	else
	{
		sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/forwarding", "all");
		fput_int(tmp, 1);
		
#if defined (USE_KERNEL3X)
		for (i=0; if6_on[i] != NULL; i++)
		{
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", if6_on[i]);
			fput_int(tmp, 0);
		}
#endif
		sprintf(tmp, "/proc/sys/net/ipv6/neigh/%s/gc_stale_time", IFNAME_BR);
		fput_int(tmp, 900); // ARP cache 15m
	}
}

void control_if_ipv6(char *ifname, int enable)
{
	char tmp[64];
	sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", ifname);
	fput_int(tmp, (enable) ? 0 : 1);
}

void control_if_ipv6_autoconf(char *ifname, int enable)
{
	char tmp[64];

	sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/accept_ra_pinfo", ifname);
	fput_int(tmp, (enable) ? 1 : 0);
}

void control_if_ipv6_radv(char *ifname, int enable)
{
	char tmp[64];

	sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/accept_ra", ifname);
	fput_int(tmp, (enable) ? 2 : 0);
	if (enable) {
		sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/forwarding", ifname);
		fput_int(tmp, 2); // Hybrid mode (Router Solicitations are being sent when necessary)
	}
}

void control_if_ipv6_dad(char *ifname, int enable)
{
	char tmp[64];

	sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/accept_dad", ifname);
	fput_int(tmp, (enable) ? 1 : 0);
	if (enable) {
		sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/dad_transmits", ifname);
		fput_int(tmp, (strcmp(ifname, IFNAME_BR) == 0) ? 2 : 1);
	}
}

void clear_if_addr6(char *ifname)
{
	doSystem("ip -6 addr flush dev %s scope global", ifname);
}

void clear_if_route6(char *ifname)
{
	doSystem("ip -6 route flush dev %s", ifname);
}

void clear_if_neigh6(char *ifname)
{
	doSystem("ip -6 neigh flush dev %s", ifname);
}

void clear_all_addr6(void)
{
	doSystem("ip -6 addr flush scope global");
}

void clear_all_route6(void)
{
	system("ip -6 route flush scope all");
}

void full_restart_ipv6(int ipv6_type_old)
{
	int ipv6_type = get_ipv6_type();
	int ipv6_toggled = ((ipv6_type == IPV6_DISABLED || ipv6_type_old == IPV6_DISABLED) && (ipv6_type != ipv6_type_old)) ? 1 : 0;

	if (ipv6_toggled) {
		stop_lltd();
		stop_httpd();
	}

	stop_radvd();
	stop_dhcp6c();
	stop_dns_dhcpd();

	if (ipv6_type == IPV6_DISABLED) {
		hwnat_configure();
		clear_all_route6();
		clear_all_addr6();
		clear_if_neigh6(IFNAME_BR);
		stop_sit_tunnel();
		reset_lan6_vars();
		reset_wan6_vars();
		control_if_ipv6_all(0);
		update_resolvconf(0, 1);
		restart_firewall();
		start_dns_dhcpd();
	}
	else {
		control_if_ipv6_all(1);
		clear_all_route6();
		clear_all_addr6();
		reset_lan6_vars();
		reload_lan_addr6();
		full_restart_wan();
		try_start_dns_dhcpd();
	}

	if (ipv6_toggled) {
		start_httpd(0);
		start_lltd();
	}
}

int ipv6_from_string(const char *str, struct in6_addr *addr6)
{
	char addr6s[INET6_ADDRSTRLEN] = "::/0";
	char *last, *tmp = addr6s;
	int ret = 128;

	if (str && *str)
		strncpy(addr6s, str, sizeof(addr6s));
	strsep(&tmp, "/");

	if (inet_pton(AF_INET6, addr6s, addr6) != 1)
		return -1;

	if (tmp && *tmp) {
		ret = strtol(tmp, &last, 10);
		if (*last || ret < 0 || ret > 128)
			ret = -1;
	}

	return ret;
}

int ipv6_to_net(struct in6_addr *addr6, int prefix)
{
	int i = prefix >> 5;
	int m = prefix & 0x1f;

	if (prefix > 128)
		return 0;

	if (m)
		addr6->s6_addr32[i++] &= htonl(0xffffffffUL << (32 - m));
	while (i < 4)
		addr6->s6_addr32[i++] = 0;

	return prefix;
}

int ipv6_to_host(struct in6_addr *addr6, int prefix)
{
	int i = prefix >> 5;
	int m = prefix & 0x1f;

	if (prefix > 128)
		return 0;

	if (m)
		addr6->s6_addr32[i--] &= htonl(0xffffffffUL >> m);
	while (i >= 0)
		addr6->s6_addr32[i--] = 0;

	return 128 - prefix;
}

int ipv6_to_ipv4_map(struct in6_addr *addr6, int size6, struct in_addr *addr4, int size4)
{
	int i = size6 >> 5;
	int m = size6 & 0x1f;
	int ret = size6 + 32 - size4;
	u_int32_t addr = 0;
	u_int32_t mask = 0xffffffffUL << size4;

	if (size6 > 128 || size4 > 32 || ret > 128)
		return 0;

	if (size4 == 32)
		return size6;

	if (addr4)
		addr = ntohl(addr4->s_addr) << size4;

	addr6->s6_addr32[i] &= ~htonl(mask >> m);
	addr6->s6_addr32[i] |= htonl(addr >> m);
	if (m) {
		i++;
		addr6->s6_addr32[i] &= ~htonl(mask << (32 - m));
		addr6->s6_addr32[i] |= htonl(addr << (32 - m));
	}

	return ret;
}

int get_prefix6_len(struct sockaddr_in6 *mask6)
{
	int i, j, prefix = 0;
	unsigned char *netmask = (unsigned char *) &(mask6)->sin6_addr;

	for (i = 0; i < 16; i++, prefix += 8)
		if (netmask[i] != 0xff)
			break;

	if (i != 16 && netmask[i])
		for (j = 7; j > 0; j--, prefix++)
			if ((netmask[i] & (1 << j)) == 0)
				break;

	return prefix;
}

#if defined (HAVE_GETIFADDRS)
char *get_ifaddr6(char *ifname, int linklocal, char *p_addr6s)
{
	char *ret = NULL;
	int prefix;
	struct ifaddrs *ifap, *ife;
	const struct sockaddr_in6 *addr6;

	if (getifaddrs(&ifap) < 0)
		return NULL;

	for (ife = ifap; ife; ife = ife->ifa_next)
	{
		if (strcmp(ifname, ife->ifa_name) != 0)
			continue;
		if (ife->ifa_addr == NULL)
			continue;
		if (ife->ifa_addr->sa_family == AF_INET6)
		{
			addr6 = (const struct sockaddr_in6 *)ife->ifa_addr;
			if (IN6_IS_ADDR_LINKLOCAL(&addr6->sin6_addr) ^ linklocal)
				continue;
			if (inet_ntop(ife->ifa_addr->sa_family, &addr6->sin6_addr, p_addr6s, INET6_ADDRSTRLEN) != NULL) {
				prefix = get_prefix6_len((struct sockaddr_in6 *)ife->ifa_netmask);
				if (prefix > 0 && prefix < 128)
					sprintf(p_addr6s, "%s/%d", p_addr6s, prefix);
				ret = p_addr6s;
				break;
			}
		}
	}
	freeifaddrs(ifap);
	return ret;
}
#else
/* getifaddrs replacement */
char *get_ifaddr6(char *ifname, int linklocal, char *p_addr6s)
{
	FILE *fp;
	char *ret = NULL;
	char addr6s[INET6_ADDRSTRLEN], addr6p[8][8], devname[32];
	int if_idx, plen, scope, scope_need, dad_status;
	struct in6_addr addr6;

	scope_need = (linklocal) ? 0x20 : 0x00;

	fp = fopen("/proc/net/if_inet6", "r");
	if (!fp)
		return NULL;
	while (fscanf(fp, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
		addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
		addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
		&dad_status, devname) != EOF)
	{
		if (strcmp(ifname, devname) != 0)
			continue;
		scope = scope & 0x00f0;
		if (scope != scope_need)
			continue;
		sprintf(addr6s, "%s:%s:%s:%s:%s:%s:%s:%s",
			addr6p[0], addr6p[1], addr6p[2], addr6p[3],
			addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
		if (inet_pton(AF_INET6, addr6s, &addr6) > 0 &&
		    inet_ntop(AF_INET6, &addr6, p_addr6s, INET6_ADDRSTRLEN) != NULL) {
			if (plen > 0 && plen < 128)
				sprintf(p_addr6s, "%s/%d", p_addr6s, plen);
			ret = p_addr6s;
			break;
		}
	}
	fclose(fp);
	return ret;
}
#endif


#endif

