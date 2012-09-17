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
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>

#include <nvram/bcmnvram.h>
#include <shutils.h>

#include "rc.h"

#if defined (USE_IPV6)

void reset_lan6_vars(void)
{
	nvram_set("lan_ip6addr_t", "");
}

int 
is_lan_radv_on(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_invmatch("ip6_lan_radv", "0"))
		return 1;
	
	return 0;
}

int 
is_lan_dhcpv6_on(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	return nvram_get_int("ip6_lan_dhcp");
}

int 
is_lan_addr6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_match("ip6_lan_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_NATIVE_STATELESS ||
	    ipv6_type == IPV6_6IN4)
		return 1;
	
	return 0;
}


void set_lan_addr6_static(void)
{
	struct in6_addr addr6;
	char addr6s[INET6_ADDRSTRLEN];
	int ip6_size;

	if (is_lan_addr6_static() == 1)
	{
		ipv6_from_string(nvram_safe_get("ip6_lan_addr"), &addr6);
		inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN);
		nvram_set("lan_ip6addr_t", addr6s);
		ip6_size = nvram_get_int("ip6_lan_size");
		if (ip6_size > 0 && ip6_size < 128)
			sprintf(addr6s, "%s/%d", addr6s, ip6_size);
		else
			ip6_size = 128;
		nvram_set_int("lan_ip6size_t", ip6_size);
		doSystem("ip -6 addr add %s dev %s", addr6s, IFNAME_BR);
	}
}

void clear_lan_addr6(void)
{
	clear_if_addr6(IFNAME_BR);
	
	nvram_set("lan_ip6addr_t", "");
	nvram_set("lan_ip6size_t", "64");
}

char *get_lan_addr6_net(char *p_addr6s)
{
	struct in6_addr addr6;
	char *lan_ip6addr_t = nvram_safe_get("lan_ip6addr_t");
	if (*lan_ip6addr_t && ipv6_from_string(lan_ip6addr_t, &addr6) >= 0) {
		ipv6_to_net(&addr6, nvram_get_int("lan_ip6size_t"));
		inet_ntop(AF_INET6, &addr6, p_addr6s, INET6_ADDRSTRLEN);
		return p_addr6s;
	}
	
	return NULL;
}


#endif
