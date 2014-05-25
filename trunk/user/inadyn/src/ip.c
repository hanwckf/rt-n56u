/* Interface for IP functions
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2010-2014  Joachim Nilsson <troglobit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, visit the Free Software Foundation
 * website at http://www.gnu.org/licenses/gpl-2.0.html or write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>

#include "debug.h"
#include "ip.h"


int ip_construct(ip_sock_t *ip)
{
	ASSERT(ip);

	memset(ip, 0, sizeof(ip_sock_t));

	ip->initialized = 0;
	ip->bound       = 0;
	ip->socket      = -1; /* Initialize to 'error', not a possible socket id. */
	ip->timeout     = IP_DEFAULT_TIMEOUT;
	memset(&ip->local_addr,  0, sizeof(ip->local_addr));
	memset(&ip->remote_addr, 0, sizeof(ip->remote_addr));

	return 0;
}

/* Resource free. */
int ip_destruct(ip_sock_t *ip)
{
	ASSERT(ip);

	if (ip->initialized == 1)
		ip_exit(ip);

	return 0;
}

/* Sets up the object. */
int ip_init(ip_sock_t *ip)
{
	int rc = 0;
	struct ifreq ifr;
	struct sockaddr_in *addrp = NULL;

	ASSERT(ip);

	if (ip->initialized == 1)
		return 0;

	do {
		TRY(os_ip_support_startup());

		/* local bind, to interface */
		if (ip->ifname) {
			int sd = socket(PF_INET, SOCK_DGRAM, 0);

			if (sd < 0) {
				int code = os_get_socket_error();

				logit(LOG_WARNING, "Failed opening network socket: %s", strerror(code));
				rc = RC_IP_OS_SOCKET_INIT_FAILED;
				break;
			}

			memset(&ifr, 0, sizeof(struct ifreq));
			strncpy(ifr.ifr_name, ip->ifname, IFNAMSIZ);
			if (ioctl(sd, SIOCGIFADDR, &ifr) != -1) {
				ip->local_addr.sin_family = AF_INET;
				ip->local_addr.sin_port = htons(0);
				addrp = (struct sockaddr_in *)&(ifr.ifr_addr);
				ip->local_addr.sin_addr.s_addr = addrp->sin_addr.s_addr;
				ip->bound = 1;

				logit(LOG_INFO, "Bound to interface %s (IP# %s)",
				      ip->ifname, inet_ntoa(ip->local_addr.sin_addr));
			} else {
				int code = os_get_socket_error();

				logit(LOG_ERR, "Failed reading IP address of interface %s: %s",
				      ip->ifname, strerror(code));
				ip->bound = 0;
			}
			close(sd);
		}

		/* remote address */
		if (ip->p_remote_host_name) {
			int s;
			char port[10];
			struct addrinfo hints, *result;

			/* Clear DNS cache before calling getaddrinfo(). */
			res_init();

			/* Obtain address(es) matching host/port */
			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = AF_INET;	/* Use AF_UNSPEC to allow IPv4 or IPv6 */
			hints.ai_socktype = SOCK_DGRAM;	/* Datagram socket */
			snprintf(port, sizeof(port), "%d", ip->port);

			s = getaddrinfo(ip->p_remote_host_name, port, &hints, &result);
			if (s != 0 || !result) {
				logit(LOG_WARNING, "Failed resolving hostname %s: %s",
				      ip->p_remote_host_name, gai_strerror(s));
				rc = RC_IP_INVALID_REMOTE_ADDR;
				break;
			}

			/* XXX: Here we should iterate over all of the records returned by
			 * getaddrinfo(), but with this code here in ip.c and connect() being
			 * in tcp.c that's hardly feasible.  Needs refactoring!  --Troglobit */
			ip->remote_addr = *result->ai_addr;
			ip->remote_len = result->ai_addrlen;

			freeaddrinfo(result);	/* No longer needed */
		}
	}
	while (0);

	if (rc) {
		ip_exit(ip);
		return rc;
	}

	ip->initialized = 1;

	return 0;
}

/* Disconnect and some other clean up. */
int ip_exit(ip_sock_t *ip)
{
	ASSERT(ip);

	if (!ip->initialized)
		return 0;

	if (ip->socket > -1) {
		close(ip->socket);
		ip->socket = -1;
	}

	os_ip_support_cleanup();

	ip->initialized = 0;

	return 0;
}

int ip_send(ip_sock_t *ip, const char *buf, int len)
{
	ASSERT(ip);

	if (!ip->initialized)
		return RC_IP_OBJECT_NOT_INITIALIZED;

	if (send(ip->socket, buf, len, 0) == -1) {
		int code = os_get_socket_error();

		logit(LOG_WARNING, "Network error while sending query/update: %s", strerror(code));
		return RC_IP_SEND_ERROR;
	}

	return 0;
}

/*
  Receive data into user's buffer.
  return
  if the max len has been received
  if a timeout occures
  In p_recv_len the total number of bytes are returned.
  Note:
  if the recv_len is bigger than 0, no error is returned.
*/
int ip_recv(ip_sock_t *ip, char *buf, int len, int *recv_len)
{
	int rc = 0;
	int remaining_bytes = len;
	int total_bytes = 0;

	ASSERT(ip);
	ASSERT(buf);
	ASSERT(recv_len);

	if (!ip->initialized)
		return RC_IP_OBJECT_NOT_INITIALIZED;

	while (remaining_bytes > 0) {
		int bytes;
		int chunk_size = remaining_bytes > IP_DEFAULT_READ_CHUNK_SIZE
			? IP_DEFAULT_READ_CHUNK_SIZE
			: remaining_bytes;

		bytes = recv(ip->socket, buf + total_bytes, chunk_size, 0);
		if (bytes < 0) {
			int code = os_get_socket_error();
			logit(LOG_WARNING, "Network error while waiting for reply: %s",
			      strerror(code));
			rc = RC_IP_RECV_ERROR;
			break;
		}

		if (bytes == 0) {
			if (total_bytes == 0)
				rc = RC_IP_RECV_ERROR;
			break;
		}

		total_bytes    += bytes;
		remaining_bytes = len - total_bytes;
	}

	*recv_len = total_bytes;

	return rc;
}

/* Accessors */
int ip_set_port(ip_sock_t *ip, int port)
{
	ASSERT(ip);

	if (port < 0 || port > IP_SOCKET_MAX_PORT)
		return RC_IP_BAD_PARAMETER;

	ip->port = port;

	return 0;
}

int ip_get_port(ip_sock_t *ip, int *port)
{
	ASSERT(ip);
	ASSERT(port);
	*port = ip->port;

	return 0;
}

int ip_set_remote_name(ip_sock_t *ip, const char *name)
{
	ASSERT(ip);
	ip->p_remote_host_name = name;

	return 0;
}

int ip_get_remote_name(ip_sock_t *ip, const char **name)
{
	ASSERT(ip);
	ASSERT(name);
	*name = ip->p_remote_host_name;

	return 0;
}

int ip_set_remote_timeout(ip_sock_t *ip, int timeout)
{
	ASSERT(ip);
	ip->timeout = timeout;

	return 0;
}

int ip_get_remote_timeout(ip_sock_t *ip, int *timeout)
{
	ASSERT(ip);
	ASSERT(timeout);
	*timeout = ip->timeout;

	return 0;
}

int ip_set_bind_iface(ip_sock_t *ip, char *ifname)
{
	ASSERT(ip);
	ip->ifname = ifname;

	return 0;
}

int ip_get_bind_iface(ip_sock_t *ip, char **ifname)
{
	ASSERT(ip);
	ASSERT(ifname);
	*ifname = ip->ifname;

	return 0;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
