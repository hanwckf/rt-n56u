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
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>

#include <include/ibox.h>
#include "infosvr.h"

#define SRV_ADDR INADDR_ANY
#define CLN_ADDR INADDR_BROADCAST

static const char* g_pidfile = "/var/run/infosvr.pid";
static int g_fd4 = -1;

int open_socket4(char *ifname)
{
	const int int_1 = 1;
	struct sockaddr_in addr;
	struct ifreq ifr;
	int fd;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto error;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr)) < 0)
		goto error;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&int_1, sizeof(int_1)) < 0)
		goto error;

	if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *)&int_1, sizeof(int_1)) < 0)
		goto error;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(SRV_ADDR);
	addr.sin_port = htons(IBOX_SRV_PORT);
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		goto error;

	return fd;

error:
	if (fd >= 0)
		close(fd);
	return -1;
}

int read_packet(int fd, void *packet, int size)
{
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	int res = 0;

	memset(packet, 0, sizeof(*packet));
	while ((res = recvfrom(fd, packet, size, 0, (struct sockaddr *)&addr , &addrlen)) < 0 && errno == EINTR)
		continue;

	return (res == size);
}

int send_packet(int fd, void *packet, int size)
{
	struct sockaddr_in addr;
	int res = 0;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(CLN_ADDR);
	addr.sin_port = htons(IBOX_CLI_PORT);

	while ((res = sendto(fd, packet, size, 0, (struct sockaddr *)&addr, sizeof(addr))) < 0 && errno == EINTR)
		continue;

	return res;
}

void *process_packet(int fd, void *packet, char *ifname)
{
	static char packet_res[512] = {0};
	IBOX_COMM_PKT_HDR_EX *hdr = packet;
	IBOX_COMM_PKT_RES_EX *hdr_res = (IBOX_COMM_PKT_RES_EX *) &packet_res;
	PKT_GET_INFO *info;

	if (hdr->ServiceID != NET_SERVICE_ID_IBOX_INFO ||
	    hdr->PacketType != NET_PACKET_TYPE_CMD)
		return NULL;

	if (hdr->OpCode != NET_CMD_ID_GETINFO && hdr->OpCode != NET_CMD_ID_GETINFO_MANU &&
	    hdr->OpCode == hdr_res->OpCode &&
	    hdr->Info == hdr_res->Info)
		return hdr_res;

	hdr_res->ServiceID = NET_SERVICE_ID_IBOX_INFO;
	hdr_res->PacketType = NET_PACKET_TYPE_RES;
	hdr_res->OpCode = hdr->OpCode;

	if (hdr->OpCode != NET_CMD_ID_GETINFO && hdr->OpCode != NET_CMD_ID_GETINFO_MANU) {
		unsigned char hwaddr[6];
		get_hwaddr(ifname, hwaddr, sizeof(hwaddr));
		if (memcmp(hdr->MacAddress, hwaddr, sizeof(hdr_res->MacAddress)) == 0)
			return NULL;
		hdr_res->Info = hdr->Info;
		memcpy(hdr_res->MacAddress, hdr->MacAddress, sizeof(hdr_res->MacAddress));
	}

	switch (hdr->OpCode) {
	case NET_CMD_ID_GETINFO:
	case NET_CMD_ID_GETINFO_MANU:
		info = (PKT_GET_INFO *)(packet_res + sizeof(IBOX_COMM_PKT_RES));
		memset(info, 0, sizeof(*info));
		get_printer(info->PrinterInfo, sizeof(info->PrinterInfo));
		get_productid(info->ProductID, sizeof(info->ProductID));
		get_firmware(info->FirmwareVersion, sizeof(info->FirmwareVersion));
		get_ssid(ifname, info->SSID, sizeof(info->SSID));
		get_netmask(ifname, info->NetMask, sizeof(info->NetMask));
		get_hwaddr(ifname, info->MacAddress, sizeof(info->MacAddress));
		info->OperationMode = get_opmode(ifname);
		info->Regulation = get_regulation(ifname);
		break;
	default:
		return NULL;
	}

	return hdr_res;
}

void res_cleanup(void)
{
	if (g_fd4 > 0) {
		close(g_fd4);
		g_fd4 = -1;
	}
	remove(g_pidfile);
}

static void catch_sig_infosvr(int sig)
{
	if (sig == SIGTERM)
	{
		res_cleanup();
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char packet[512], *reply, *ifname;
	int max_fd, res;
	fd_set rfds, active_rfds;

	if (!argv[1]) {
		fprintf(stderr, "usage: %s {ifname}\n", argv[0]);
		return 1;
	}

	ifname = argv[1];

	g_fd4 = open_socket4(ifname);
	if (g_fd4 < 0) {
		perror("open_socket");
		return 1;
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGHUP,  SIG_IGN);
	signal(SIGTERM, catch_sig_infosvr);

	if (daemon(0, 0) < 0) {
		close(g_fd4);
		perror("daemon");
		exit(errno);
	}

	/* write pid */
	if ((fp=fopen(g_pidfile, "w"))!=NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	FD_ZERO(&rfds);
	FD_SET(g_fd4, &rfds);
	max_fd = g_fd4;

	while (1) {
		active_rfds = rfds;
		while ((res = select(max_fd + 1, &active_rfds, NULL, NULL, NULL)) < 0 && 
		       (errno == EINTR || errno == EAGAIN))
			continue;
		
		if (res < 0)
			break;
		
		if (res == 0 || !FD_ISSET(g_fd4, &active_rfds))
			continue;
		
		if (read_packet(g_fd4, packet, sizeof(packet)) < 0)
			continue;
		
		reply = process_packet(g_fd4, packet, ifname);
		if (reply)
			send_packet(g_fd4, reply, sizeof(packet));
	}

	res_cleanup();

	return 0;
}
