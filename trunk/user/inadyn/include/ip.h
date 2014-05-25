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
 * Boston, MA  02110-1301, USA.
 */

#ifndef INADYN_IP_H_
#define INADYN_IP_H_

#include "os.h"
#include "error.h"

#define IP_DEFAULT_TIMEOUT		20000	/* msec */
#define IP_SOCKET_MAX_PORT		65535
#define IP_DEFAULT_READ_CHUNK_SIZE	100

enum {
	TYPE_TCP = 0,
	TYPE_UDP
};

typedef struct {
	int                 initialized;

	char               *ifname;
	int                 bound;		/* When bound to an interface */

	int                 type;
	int                 socket;
	struct sockaddr_in  local_addr;
	struct sockaddr     remote_addr;
	socklen_t           remote_len;
	const char         *p_remote_host_name;

	unsigned short      port;
	int                 timeout;
} ip_sock_t;

int ip_construct          (ip_sock_t *ip);
int ip_destruct           (ip_sock_t *ip);

int ip_init               (ip_sock_t *ip);
int ip_exit               (ip_sock_t *ip);

int ip_send               (ip_sock_t *ip, const char *buf, int len);
int ip_recv               (ip_sock_t *ip,       char *buf, int len, int *recv_len);

int ip_set_port           (ip_sock_t *ip, int  port);
int ip_get_port           (ip_sock_t *ip, int *port);

int ip_set_remote_name    (ip_sock_t *ip, const char  *name);
int ip_get_remote_name    (ip_sock_t *ip, const char **name);

int ip_set_remote_timeout (ip_sock_t *ip, int  timeout);
int ip_get_remote_timeout (ip_sock_t *ip, int *timeout);

int ip_set_bind_iface     (ip_sock_t *ip, char  *ifname);
int ip_get_bind_iface     (ip_sock_t *ip, char **ifname);

#endif /* INADYN_IP_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
