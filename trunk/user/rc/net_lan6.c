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
	struct in6_addr addr6;
	char addr6s[INET6_ADDRSTRLEN] = {0};
	char *lan_addr6;
	int lan_size6;
	
	if (is_lan_addr6_static() == 1)
	{
		lan_addr6 = nvram_safe_get("ip6_lan_addr");
		lan_size6 = nvram_get_int("ip6_lan_size");
		if (*lan_addr6) {
			memset(&addr6, 0, sizeof(addr6));
			ipv6_from_string(lan_addr6, &addr6);
			inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN);
			if (lan_size6 > 0 && lan_size6 < 128)
				sprintf(addr6s, "%s/%d", addr6s, lan_size6);
		}
	}

	nvram_set("lan_addr6", addr6s);
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
is_lan_dhcp6s_on(void)
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

	if (ipv6_type == IPV6_6TO4 ||
	    ipv6_type == IPV6_6RD)
		return 0;

	if (nvram_match("ip6_lan_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_6IN4)
		return 1;
	
	return 0;
}

int update_lan_addr6_radv(char *lan_addr6_new)
{
	char *lan_addr6_old;
	
	if (!lan_addr6_new)
		return 0;
	
	if (!(*lan_addr6_new))
		return 0;
	
	lan_addr6_old = nvram_safe_get("lan_addr6");
	if (strcmp(lan_addr6_new, lan_addr6_old) != 0) {
		nvram_set("lan_addr6", lan_addr6_new);
		if (nvram_invmatch("ip6_lan_radv", "0") || nvram_invmatch("ip6_lan_dhcp", "0"))
			return 1;
	}

	return 0;
}

void reload_lan_addr6(void)
{
	char *lan_addr6;
	
	clear_if_addr6(IFNAME_BR);
	lan_addr6 = nvram_safe_get("lan_addr6");
	if (*lan_addr6)
		doSystem("ip -6 addr add %s dev %s", lan_addr6, IFNAME_BR);
}

void clear_lan_addr6(void)
{
	clear_if_addr6(IFNAME_BR);
}

const char *get_lan_addr6_net(char *p_addr6s)
{
	int lan_size6;
	struct in6_addr addr6;
	char *lan_addr6 = get_ifaddr6(IFNAME_BR, 0, p_addr6s);
	if (lan_addr6) {
		lan_size6 = ipv6_from_string(lan_addr6, &addr6);
		if (lan_size6 >= 0) {
			ipv6_to_net(&addr6, lan_size6);
			return inet_ntop(AF_INET6, &addr6, p_addr6s, INET6_ADDRSTRLEN);
		}
	}
	
	return NULL;
}

#endif
