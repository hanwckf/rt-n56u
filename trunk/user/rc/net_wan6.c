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
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <nvram/bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <signal.h>

#include "rc.h"

#if defined (USE_IPV6)

void reset_wan6_vars(void)
{
	struct in6_addr addr6;
	char addr6s[INET6_ADDRSTRLEN] = "::/0";
	int ip6_size;

	memset(&addr6, 0, sizeof(addr6));
	ipv6_from_string(nvram_safe_get("ip6_wan_addr"), &addr6);
	inet_ntop(AF_INET6, &addr6, addr6s, INET6_ADDRSTRLEN);
	ip6_size = nvram_get_int("ip6_wan_size");
	if (ip6_size > 0 && ip6_size < 128)
		sprintf(addr6s, "%s/%d", addr6s, ip6_size);

	nvram_set("wan0_ip6_wan_addr", addr6s);
	nvram_set("wan0_ip6_wan_gate", nvram_safe_get("ip6_wan_gate"));
	nvram_set("wan0_ip6_6rd_gate", nvram_safe_get("ip6_6rd_relay"));
	nvram_set("wan0_ip6_6rd_size", nvram_safe_get("ip6_6rd_size"));

	nvram_set("wan0_ip6_dns1", nvram_safe_get("ip6_dns1"));
	nvram_set("wan0_ip6_dns2", nvram_safe_get("ip6_dns2"));
	nvram_set("wan0_ip6_dns3", nvram_safe_get("ip6_dns3"));
}

int 
is_wan_addr6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_match("ip6_wan_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_6IN4)
		return 1;
	
	return 0;
}

int 
is_wan_dns6_static(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (nvram_match("ip6_dns_auto", "0") || 
	    ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_NATIVE_STATELESS ||
	    ipv6_type == IPV6_6IN4)
		return 1;
	
	return 0;
}

int 
is_ipoe6_allowed(void)
{
	int ipv6_type = get_ipv6_type();

	if (ipv6_type == IPV6_DISABLED)
		return -1;

	if (ipv6_type == IPV6_NATIVE_STATIC || 
	    ipv6_type == IPV6_NATIVE_STATELESS || 
	    ipv6_type == IPV6_NATIVE_DHCP6)
		return 1;

	return 0;
}

void set_wan_addr6_static(void)
{
	if (is_wan_addr6_static() == 1)
	{
		doSystem("ip -6 addr add %s dev %s", nvram_safe_get("wan0_ip6_wan_addr"), IFNAME_WAN);
	}
}

void store_ip6rd_from_dhcp(const char *env_value, const char *prefix)
{
	int i;
	char tmp[100];
	char ip6rd[64];
	char addr6[INET6_ADDRSTRLEN];
	char *value, *values[4];

	strncpy(ip6rd, env_value, sizeof(ip6rd));
	value = ip6rd;
	for (i = 0; i < 4 && value; i++)
		values[i] = strsep(&value, " ");
	if (i == 4)
	{
		snprintf(addr6, sizeof(addr6), "%s/%s", values[2], values[1]);
		nvram_set(strcat_r(prefix, "ip6_wan_addr", tmp), addr6);
		nvram_set(strcat_r(prefix, "ip6_6rd_size", tmp), values[0]);
		nvram_set(strcat_r(prefix, "ip6_6rd_gate", tmp), values[3]);
	}
}

void wan6_up(char *wan_ifname)
{
	int ipv6_type;
	char *wan6_ifname;
	char *wan6_ipaddr, *wan4_ipaddr;
	char *sit_remote;
	int sit_ttl, sit_mtu;
	char tmp[100], prefix[16];

	ipv6_type = get_ipv6_type();
	if (ipv6_type == IPV6_DISABLED)
		return;

	if (wan_prefix(wan_ifname, prefix) < 0)
		return;

	wan4_ipaddr = nvram_safe_get(strcat_r(prefix, "ipaddr", tmp));

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
	{
		wan6_ifname = IFNAME_SIT;
		sit_mtu = nvram_get_int("ip6_sit_mtu");
		sit_ttl = nvram_get_int("ip6_sit_ttl");
		sit_remote = "any";
		if (ipv6_type == IPV6_6IN4)
			sit_remote = nvram_safe_get("ip6_6in4_remote");
		else if ( ipv6_type == IPV6_6TO4)
			sit_remote = nvram_safe_get("ip6_6to4_relay");
		
		doSystem("ip tunnel add %s mode sit remote %s local %s ttl %d", IFNAME_SIT, sit_remote, wan4_ipaddr, sit_ttl);
		
		doSystem("ip link set mtu %d dev %s up", sit_mtu, IFNAME_SIT);
	}
	else
	{
		wan6_ifname = wan_ifname;
	}
	
	// todo
}

void wan6_down(char *wan_ifname)
{
	int ipv6_type;
	char tmp[100], prefix[16];
	char *wan6_ifname, *wan6_gate;

	ipv6_type = get_ipv6_type();
	if (ipv6_type == IPV6_DISABLED)
		return;

	if (wan_prefix(wan_ifname, prefix) < 0)
		return;

	stop_dhcp6c();

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
	{
		wan6_ifname = IFNAME_SIT;
		doSystem("ip link set dev %s down", IFNAME_SIT);
	}
	else
	{
		wan6_ifname = wan_ifname;
	}

	if (ipv6_type != IPV6_6TO4 && ipv6_type != IPV6_6RD)
	{
		wan6_gate = nvram_safe_get(strcat_r(prefix, "ip6_wan_gate", tmp));
		if (*wan6_gate)
			doSystem("ip -6 route del %s dev %s", wan6_gate, wan6_ifname);
	}

	doSystem("ip -6 route del default metric 1");

	clear_if_addr6(wan6_ifname);

	if (ipv6_type == IPV6_6IN4 || ipv6_type == IPV6_6TO4 || ipv6_type == IPV6_6RD)
	{
		doSystem("ip tunnel del %s", IFNAME_SIT);
	}

	// todo
}

int dhcp6c_main(int argc, char **argv)
{
	// todo

	return 0;
}

int start_dhcp6c(char *wan_ifname)
{
	// todo

	return 0;
}

void stop_dhcp6c(void)
{
	kill_pidfile_s("/var/run/dhcp6c.pid", SIGTERM);
}

#endif

