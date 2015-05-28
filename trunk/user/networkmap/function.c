#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/socket.h>

#include <netutils.h>
#include <include/ibox.h>

#include "networkmap.h"

extern unsigned int scan_now;


/***** ASUS routers detect function *****/
static int
asus_dd_query(struct in_addr *dst_ip, NET_CLIENT *pnet_client)
{
	char buffer[512];
	struct sockaddr_in own_saddr, dst_saddr;
	struct timeval timeout;
	int sock_dd, recvlen, retry, dd_result, flag_one;
	IBOX_COMM_PKT_HDR_EX dd_hdr;

	sock_dd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_dd < 0)
	{
		NMP_DEBUG("DD: socket error.\n");
		return -1;
	}

	flag_one = 1;
	setsockopt(sock_dd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag_one, sizeof(flag_one));

	memset(&own_saddr, 0, sizeof(own_saddr));
	own_saddr.sin_family = AF_INET;
	own_saddr.sin_port = htons(IBOX_CLI_PORT);
	own_saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock_dd, (struct sockaddr *)&own_saddr, sizeof(own_saddr)) < 0) {
		close(sock_dd);
		NMP_DEBUG_M("DD: bind error!\n");
		return -1;
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;
	setsockopt(sock_dd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock_dd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	/* initialize value in dest */
	memset(&dst_saddr, 0, sizeof(dst_saddr));
	memcpy(&dst_saddr.sin_addr, dst_ip, sizeof(struct in_addr));
	dst_saddr.sin_family = AF_INET;
	dst_saddr.sin_port = htons(IBOX_SRV_PORT);

	/* fill discovery header */
	memset(&dd_hdr, 0, sizeof(dd_hdr));
	dd_hdr.ServiceID = NET_SERVICE_ID_IBOX_INFO;
	dd_hdr.PacketType = NET_PACKET_TYPE_CMD;
	dd_hdr.OpCode = NET_CMD_ID_GETINFO;

	dd_result = 0;

	for (retry = 0; retry < 2; retry++) {
		if (sendto(sock_dd, (char *)&dd_hdr, sizeof(dd_hdr), 0, (struct sockaddr*)&dst_saddr, sizeof(dst_saddr)) == -1) {
			NMP_DEBUG_M("DD: send error!\n");
			break;
		}
		
		bzero(buffer, sizeof(buffer));
		recvlen = recv(sock_dd, buffer, sizeof(buffer), 0);
		if (recvlen >= (int)(sizeof(IBOX_COMM_PKT_RES)+sizeof(PKT_GET_INFO))) {
			IBOX_COMM_PKT_RES_EX *dd_res = (IBOX_COMM_PKT_RES_EX *)buffer;
			
			if (dd_res->ServiceID == NET_SERVICE_ID_IBOX_INFO &&
			    dd_res->PacketType == NET_PACKET_TYPE_RES &&
			    dd_res->OpCode == NET_CMD_ID_GETINFO) {
				PKT_GET_INFO *dd_info = (PKT_GET_INFO *)(buffer+sizeof(IBOX_COMM_PKT_RES));
				
				dd_result = 1;
				
				pnet_client->type = 3; // ASUS AP
				
				NMP_DEBUG_M("DD: productID=%s\n", dd_info->ProductID);
				if (!pnet_client->device_name[0] && is_valid_hostname(dd_info->ProductID)) {
					memcpy(pnet_client->device_name, dd_info->ProductID, 18);
					pnet_client->device_name[18] = 0;
				}
				
				break;
			}
		}
		
		usleep(100000);
	}

	close(sock_dd);

	if (!dd_result) {
		NMP_DEBUG_M("DD timeout...\n");
		return -1;
	}

	return 0;
}

/***** HTTP Server detect function *****/
static int
http_query(struct in_addr *dst_ip, NET_CLIENT *pnet_client)
{
	int sock_http, recvlen;
	struct sockaddr_in dst_saddr;
	struct timeval timeout = {1, 0};
	char buffer[1024] = {0};

	/* create socket */
	sock_http = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_http < 0)
	{
		NMP_DEBUG("HTTP: socket error.\n");
		return -1;
	}

	/* initialize value in dest */
	memset(&dst_saddr, 0, sizeof(dst_saddr));
	memcpy(&dst_saddr.sin_addr, dst_ip, sizeof(struct in_addr));
	dst_saddr.sin_family = AF_INET;
	dst_saddr.sin_port = htons(HTTP_PORT);

	setsockopt(sock_http, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock_http, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	/* Connecting to server */
	if (connect(sock_http, (struct sockaddr*)&dst_saddr, sizeof(dst_saddr))== -1)
	{
		close(sock_http);
		NMP_DEBUG_M("HTTP: connect error!\n");
		return -1;
	}

	snprintf(buffer, sizeof(buffer), "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", inet_ntoa(dst_saddr.sin_addr));
	if (send(sock_http, buffer, strlen(buffer), 0) == -1)
	{
		close(sock_http);
		NMP_DEBUG_M("HTTP: send error!\n");
		return -1;
	}

	bzero(buffer, sizeof(buffer));
	recvlen = recv(sock_http, buffer, sizeof(buffer)-1, 0);
	if (recvlen > 12)
	{
		NMP_DEBUG_M("Check http response: %s\n", buffer);
		if (!memcmp(buffer, "HTTP/1.", 7) &&
		   (!memcmp(buffer+9, "2", 1) || !memcmp(buffer+9, "3", 1) || !memcmp(buffer+9, "401", 3)) )
		{
			NMP_DEBUG("Found HTTP!\n");
			pnet_client->http = 1;
		}
	}

	close(sock_http);

	return 0;
}

/***** NBNS Name Query function *****/
static int
nbns_query(struct in_addr *src_ip, struct in_addr *dst_ip, NET_CLIENT *pnet_client)
{
	struct sockaddr_in own_saddr, dst_saddr, other_addr2;
	int sock_nbns, recvlen, retry, nbns_result, flag_one;
	socklen_t other_addr_len2;
	struct timeval timeout;
	char recvbuf[512], device_name[18];
	char sendbuf[50] = {0x87, 0x96, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x20, 0x43, 0x4b, 0x41,
			    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
			    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
			    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
			    0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x00, 0x21,
			    0x00, 0x01};

	sock_nbns = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_nbns < 0)
	{
		NMP_DEBUG_M("NBNS: socket error.\n");
		return -1;
	}

	flag_one = 1;
	setsockopt(sock_nbns, SOL_SOCKET, SO_REUSEADDR, (char*)&flag_one, sizeof(flag_one));

	memset(&own_saddr, 0, sizeof(own_saddr));
	memcpy(&own_saddr.sin_addr, src_ip, sizeof(struct in_addr));
	own_saddr.sin_family = AF_INET;
	own_saddr.sin_port = htons(NBNS_PORT);

	if (bind(sock_nbns, (struct sockaddr *)&own_saddr, sizeof(own_saddr)) < 0)
	{
		NMP_DEBUG_M("NBNS: bind error.\n");
		close(sock_nbns);
		return -1;
	}

	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;
	setsockopt(sock_nbns, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock_nbns, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	memset(&dst_saddr, 0, sizeof(dst_saddr));
	memcpy(&dst_saddr.sin_addr, dst_ip, sizeof(struct in_addr));
	dst_saddr.sin_family = AF_INET;
	dst_saddr.sin_port = htons(NBNS_PORT);

	nbns_result = 0;
	bzero(device_name, sizeof(device_name));

	for (retry = 0; retry < 3; retry++)
	{
		if (sendto(sock_nbns, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&dst_saddr, sizeof(dst_saddr)) == -1) {
			NMP_DEBUG_M("NBNS: send error!\n");
			break;
		}
		
		other_addr_len2 = sizeof(other_addr2);
		memset(&other_addr2, 0, sizeof(other_addr2));
		bzero(recvbuf, sizeof(recvbuf));
		recvlen = recvfrom(sock_nbns, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&other_addr2, &other_addr_len2);
		if (recvlen > 74)
		{
			NBNS_RESPONSE *nbns_rsp = (NBNS_RESPONSE *)recvbuf;
			if (((nbns_rsp->flags[0]>>4) == 8) && (nbns_rsp->number_of_names > 0) &&
			     (other_addr2.sin_addr.s_addr == dst_saddr.sin_addr.s_addr))
			{
				nbns_result = 1;
				
				// check Group indicator (0x80)
				if ( !(nbns_rsp->name_flags1[0] & 0x80) )
					memcpy(device_name, nbns_rsp->device_name1, 16);
				else if ( nbns_rsp->number_of_names > 1 && !(nbns_rsp->name_flags2[0] & 0x80) )
					memcpy(device_name, nbns_rsp->device_name2, 16);
				else if ( nbns_rsp->number_of_names > 2 && !(nbns_rsp->name_flags3[0] & 0x80) )
					memcpy(device_name, nbns_rsp->device_name3, 16);
				else if ( nbns_rsp->number_of_names > 3 && !(nbns_rsp->name_flags4[0] & 0x80) )
					memcpy(device_name, nbns_rsp->device_name4, 16);
				
				break;
			}
		}
		
		usleep(100000);
	}

	close(sock_nbns);

	if (!nbns_result) {
		NMP_DEBUG_M("NBNS timeout...\n");
		return -1;
	}

	if (pnet_client->type == 6)
		pnet_client->type = 1; // PC

	if (device_name[0]) {
		device_name[17] = 0;
		NMP_DEBUG("NBNS Name: %s\n", device_name);
		if (!pnet_client->device_name[0] && is_valid_hostname(device_name))
			memcpy(pnet_client->device_name, device_name, 18);
	} else {
		NMP_DEBUG("NBNS: NO hostname!\n");
	}

	return 0;
}

void
find_all_app(struct in_addr *src_ip, struct in_addr *dst_ip, NET_CLIENT *pnet_client)
{
	/* check ASUS AP */
	asus_dd_query(dst_ip, pnet_client);

	if (scan_now == 0)
		return;

	/* check SMB server */
	if (pnet_client->type == 6 || !pnet_client->device_name[0])
		nbns_query(src_ip, dst_ip, pnet_client);

	if (scan_now == 0)
		return;

	/* check http server */
	http_query(dst_ip, pnet_client);
}

