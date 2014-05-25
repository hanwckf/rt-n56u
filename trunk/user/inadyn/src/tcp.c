/* Interface for TCP functions
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
 * Boston, MA  02110-1301, USA.
*/

#include <poll.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "tcp.h"

int tcp_construct(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	DO(ip_construct(&tcp->ip));

	/* Reset its part of the struct (skip IP part) */
	memset(((char *)tcp + sizeof(tcp->ip)), 0, sizeof(*tcp) - sizeof(tcp->ip));
	tcp->initialized = 0;

	return 0;
}

int tcp_destruct(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	if (tcp->initialized == 1)
		tcp_exit(tcp);

	return ip_destruct(&tcp->ip);
}

static int local_set_params(tcp_sock_t *tcp)
{
	int timeout;

	/* Set default TCP specififc params */
	tcp_get_remote_timeout(tcp, &timeout);
	if (timeout == 0)
		tcp_set_remote_timeout(tcp, TCP_DEFAULT_TIMEOUT);

	return 0;
}

/* Check for socket error */
static int soerror(int sd)
{
	int code = 0;
	socklen_t len = sizeof(code);

	if (getsockopt(sd, SOL_SOCKET, SO_ERROR, &code, &len))
		return 1;

	errno = code;

	return code;
}

/* In the wonderful world of network programming the manual states that
 * EINPROGRESS is only a possible error on non-blocking sockets.  Real world
 * experience, however, suggests otherwise.  Simply poll() for completion and
 * then continue. --Joachim */
static int check_error(int sd, int msec)
{
	struct pollfd pfd = { sd, POLLOUT, 0 };

	if (EINPROGRESS == errno) {
		logit(LOG_INFO, "Waiting (%d sec) for three-way handshake to complete ...", msec / 1000);
		if (poll (&pfd, 1, msec) > 0 && !soerror(sd)) {
			logit(LOG_INFO, "Connected.");
			return 0;
		}
	}

	return 1;
}

/* On error tcp_exit() is called by upper layers. */
int tcp_init(tcp_sock_t *tcp, char *msg, int verbose)
{
	int rc = 0;
	char host[NI_MAXHOST];
	struct timeval sv;
	struct sockaddr sa;
	socklen_t salen;

	ASSERT(tcp);

	do {
		int sd;

		TRY(local_set_params(tcp));
		TRY(ip_init(&tcp->ip));

		if (tcp->ip.type != TYPE_TCP)
			return RC_IP_BAD_PARAMETER;

		sd = socket(AF_INET, SOCK_STREAM, 0);
		if (sd == -1) {
			logit(LOG_ERR, "Error creating client socket: %s", strerror(errno));
			rc = RC_IP_SOCKET_CREATE_ERROR;
			break;
		}

		/* Call to socket() OK, allow tcp_exit() to run to
		 * prevent socket leak if any of the below calls fail. */
		tcp->ip.socket = sd;
		tcp->initialized  = 1;

		if (tcp->ip.bound == 1) {
			if (bind(sd, (struct sockaddr *)&tcp->ip.local_addr, sizeof(struct sockaddr_in)) < 0) {
				logit(LOG_WARNING, "Failed binding client socket to local address: %s", strerror(errno));
				rc = RC_IP_SOCKET_BIND_ERROR;
				break;
			}
		}

		/* Attempt to set TCP timers, silently fall back to OS defaults */
		sv.tv_sec  =  tcp->ip.timeout / 1000;
		sv.tv_usec = (tcp->ip.timeout % 1000) * 1000;
		setsockopt(tcp->ip.socket, SOL_SOCKET, SO_RCVTIMEO, &sv, sizeof(sv));
		setsockopt(tcp->ip.socket, SOL_SOCKET, SO_SNDTIMEO, &sv, sizeof(sv));

		sa    = tcp->ip.remote_addr;
		salen = tcp->ip.remote_len;
		if (!getnameinfo(&sa, salen, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) {
			if (verbose > 0)
				logit(LOG_INFO, "%s, connecting to %s (%s:%d)", msg, tcp->ip.p_remote_host_name, host, tcp->ip.port);
		}
		else
			logit(LOG_ERR, "%s, failed resolving %s!", msg, host);

		if (connect(sd, &sa, salen)) {
			if (!check_error(sd, tcp->ip.timeout))
				break; /* OK */

			logit(LOG_WARNING, "Failed connecting to remote server: %s", strerror(errno));
			rc = RC_IP_CONNECT_FAILED;
			break;
		}
	} while (0);

	if (rc) {
		tcp_exit(tcp);
		return rc;
	}

	return 0;
}

int tcp_exit(tcp_sock_t *tcp)
{
	ASSERT(tcp);

	if (!tcp->initialized)
		return 0;

	tcp->initialized = 0;

	return ip_exit(&tcp->ip);
}

int tcp_send(tcp_sock_t *tcp, const char *buf, int len)
{
	ASSERT(tcp);

	if (!tcp->initialized)
		return RC_TCP_OBJECT_NOT_INITIALIZED;

	return ip_send(&tcp->ip, buf, len);
}

int tcp_recv(tcp_sock_t *tcp, char *buf, int buf_len, int *recv_len)
{
	ASSERT(tcp);

	if (!tcp->initialized)
		return RC_TCP_OBJECT_NOT_INITIALIZED;

	return ip_recv(&tcp->ip, buf, buf_len, recv_len);
}

int tcp_set_port(tcp_sock_t *tcp, int port)
{
	ASSERT(tcp);

	return ip_set_port(&tcp->ip, port);
}

int tcp_get_port(tcp_sock_t *tcp, int *port)
{
	ASSERT(tcp);

	return ip_get_port(&tcp->ip, port);
}

int tcp_set_remote_name(tcp_sock_t *tcp, const char *p)
{
	ASSERT(tcp);

	return ip_set_remote_name(&tcp->ip, p);
}

int tcp_get_remote_name(tcp_sock_t *tcp, const char **p)
{
	ASSERT(tcp);

	return ip_get_remote_name(&tcp->ip, p);
}

int tcp_set_remote_timeout(tcp_sock_t *tcp, int p)
{
	ASSERT(tcp);

	return ip_set_remote_timeout(&tcp->ip, p);
}

int tcp_get_remote_timeout(tcp_sock_t *tcp, int *p)
{
	ASSERT(tcp);

	return ip_get_remote_timeout(&tcp->ip, p);
}

int tcp_set_bind_iface(tcp_sock_t *tcp, char *ifname)
{
	ASSERT(tcp);

	return ip_set_bind_iface(&tcp->ip, ifname);
}

int tcp_get_bind_iface(tcp_sock_t *tcp, char **ifname)
{
	ASSERT(tcp);
	ASSERT(ifname);

	return ip_get_bind_iface(&tcp->ip, ifname);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
