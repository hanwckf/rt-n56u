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

void init_ipv6(void)
{
	int ipv6_type = get_ipv6_type();
	control_if_ipv6((ipv6_type == IPV6_DISABLED) ? 0 : 1);
}

void control_if_ipv6(int enable)
{
	int i;
	char tmp[64];
	char* if6_on[] = { "default", "lo", IFNAME_BR, IFNAME_WAN, NULL };
	char* if6_off[] = { "default", "all", NULL };
	
	if (!enable)
	{
		for (i=0; if6_off[i] != NULL; i++)
		{
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/forwarding", if6_off[i]);
			fput_int(tmp, 0);
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", if6_off[i]);
			fput_int(tmp, 1);
		}
	}
	else
	{
		for (i=0; if6_on[i] != NULL; i++)
		{
			sprintf(tmp, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", if6_on[i]);
			fput_int(tmp, 0);
		}
	}
}

void clear_if_addr6(char *ifname)
{
	doSystem("ip -6 addr flush dev %s scope global", ifname);
}

void clear_all_addr6(char* scope)
{
	doSystem("ip -6 addr flush scope %s", scope);
}

void clear_all_route6(char* scope)
{
	doSystem("ip -6 route flush scope %s", scope);
}

void full_restart_ipv6(int ipv6_type_old)
{
	int ipv6_type = get_ipv6_type();
	int ipv6_toggled = ((ipv6_type == IPV6_DISABLED || ipv6_type_old == IPV6_DISABLED) && (ipv6_type != ipv6_type_old)) ? 1 : 0;

	stop_dns_dhcpd();

	if (ipv6_toggled) {
		stop_lltd();
		stop_httpd();
	}

	if (ipv6_type != IPV6_DISABLED) {
		control_if_ipv6(1);
		clear_all_addr6("global");
		set_lan_addr6_static();
		set_wan_addr6_static();
	}

	if (ipv6_type == IPV6_DISABLED) {
		clear_all_addr6("global");
		clear_all_route6("all");
		doSystem("ip -6 neigh flush dev %s", IFNAME_BR);
		control_if_ipv6(0);
	}

	update_resolvconf(0, 1);
	start_dns_dhcpd();

	if (ipv6_toggled) {
		start_httpd(0);
		startup_lltd();
	}

	restart_firewall();
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

#endif

