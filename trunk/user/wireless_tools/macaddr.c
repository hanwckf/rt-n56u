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
/*
 *	macaddr
 *
 *	Program to return the MAC address of an Ethernet
 *	adapter.  This was written to help configure the
 *	adapter based on the MAC address rather than the
 *	name.
 *
 *	Version 1.0	Eric Dittman	2001-10-19
 *
 *	This is released unther the GPL license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "iwlib.h"

int main(int argc, char** argv)
{

	int devsock;
	struct ifreq ifbuffer;
	char buf[20];

	if ((argc != 2) || (argv[1][0] == '-')) {
		printf("Usage: macaddr interface\n");
		exit(1);
	}

	devsock = socket(AF_INET, SOCK_STREAM, 0);

	if (devsock == -1) {
		perror("Failed opening socket");
		exit (1);
	}

	memset(&ifbuffer, 0, sizeof(ifbuffer));
	strncpy(ifbuffer.ifr_name, argv[1], sizeof(ifbuffer.ifr_name));
	if (ioctl(devsock, SIOCGIFHWADDR, &ifbuffer) == -1) {
		fprintf(stderr, "There is no MACADDR for %s\n", argv[1]);
		exit(1);
	}
	close(devsock);

	puts(iw_saether_ntop(&ifbuffer.ifr_ifru.ifru_hwaddr, buf));

	exit(0);
}
