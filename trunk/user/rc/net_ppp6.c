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

#if defined (USE_IPV6)

int 
is_ppp6_allowed(void)
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

int
ipv6up_main(int argc, char **argv)
{
	// todo

	return 0;
}

int
ipv6down_main(int argc, char **argv)
{
	// todo

	return 0;
}
#endif

