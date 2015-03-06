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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include "infosvr.h"
#include <include/ibox.h>
#include <nvram_linux.h>
#include <shutils.h>

char *get_printer(void *buf, int size)
{
	snprintf(buf, size, "%s", nvram_safe_get("printer_model_t"));
	return buf;
}

char *get_productid(void *buf, int size)
{
	snprintf(buf, size, "%s", nvram_safe_get("productid"));
	return buf;
}

char *get_firmware(void *buf, int size)
{
	snprintf(buf, size, "%s", nvram_safe_get("firmver_sub"));
	return buf;
}

char *get_ssid(char *ifname, void *buf, int size)
{
	snprintf(buf, size, "%s", nvram_safe_get("rt_ssid"));
	return buf;
}

char *get_netmask(char *ifname, void *buf, int size)
{
	snprintf(buf, size, "%s", nvram_safe_get("lan_netmask_t"));
	return buf;
}

unsigned char *get_hwaddr(char *ifname, void *buf, int size)
{
	char *macaddr = nvram_safe_get("il0macaddr");
	if (strlen(macaddr) == 0)
		return NULL;

	ether_atoe(macaddr, buf);
	return buf;
}

int get_opmode(char *ifname)
{
	return OPERATION_MODE_WB;
}

int get_regulation(char *ifname)
{
	return 0xff;
}
