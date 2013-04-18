/*
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define MODULE_TAG ""
#include <stdlib.h>
#include <string.h>

#include "debug_if.h"
#include "tcp.h"

/* basic resource allocations for the tcp object */
RC_TYPE tcp_construct(TCP_SOCKET *p_self)
{
	RC_TYPE rc;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	rc = ip_construct(&p_self->super);
	if (rc != RC_OK)
	{
		return rc;
	}

	/*reset its part of the struct (skip IP part)*/
	memset(((char*)p_self + sizeof(p_self->super)) , 0, sizeof(*p_self) - sizeof(p_self->super));
	p_self->initialized = FALSE;

	return RC_OK;
}

/*
  Resource free.
*/
RC_TYPE tcp_destruct(TCP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_OK;
	}

	if (p_self->initialized == TRUE)
	{
		tcp_shutdown(p_self);
	}

	return ip_destruct(&p_self->super);
}

static RC_TYPE local_set_params(TCP_SOCKET *p_self)
{
	int timeout;

	/* Set default TCP specififc params */
	tcp_get_remote_timeout(p_self, &timeout);

	if (timeout == 0)
	{
		tcp_set_remote_timeout(p_self, TCP_DEFAULT_TIMEOUT);
	}

	return RC_OK;
}

/*
  Sets up the object.

  - ...
*/
RC_TYPE tcp_initialize(TCP_SOCKET *p_self, char *msg)
{
	RC_TYPE rc;
	struct timeval sv;
	int svlen = sizeof(sv);
	char host[NI_MAXHOST];

	do
	{
		local_set_params(p_self);

		/*call the super*/
		rc = ip_initialize(&p_self->super);
		if (rc != RC_OK)
		{
			break;
		}

		/* local object initalizations */
		if (p_self->super.type == TYPE_TCP)
		{
			p_self->super.socket = socket(AF_INET, SOCK_STREAM, 0);
			if (p_self->super.socket == -1)
			{
				int code = os_get_socket_error();

				logit(LOG_ERR, MODULE_TAG "Error creating client socket: %s", strerror(code));
				rc = RC_IP_SOCKET_CREATE_ERROR;
				break;
			}

			/* Call to socket() OK, allow tcp_shutdown() to run to
			 * prevent socket leak if any of the below calls fail. */
			p_self->initialized = TRUE;

			if (p_self->super.bound == TRUE)
			{
				if (bind(p_self->super.socket, (struct sockaddr *)&p_self->super.local_addr, sizeof(struct sockaddr_in)) < 0)
				{
					int code = os_get_socket_error();

					logit(LOG_WARNING, MODULE_TAG "Failed binding client socket to local address: %s", strerror(code));
					rc = RC_IP_SOCKET_BIND_ERROR;
					break;
				}
			}
		}
		else
		{
			p_self->initialized = TRUE; /* Allow tcp_shutdown() to run. */
			rc = RC_IP_BAD_PARAMETER;
		}

		/* set timeouts */
		sv.tv_sec  = p_self->super.timeout / 1000;	    /* msec to sec */
		sv.tv_usec = (p_self->super.timeout % 1000) * 1000; /* reminder to usec */
		setsockopt(p_self->super.socket, SOL_SOCKET, SO_RCVTIMEO, &sv, svlen);
		setsockopt(p_self->super.socket, SOL_SOCKET, SO_SNDTIMEO, &sv, svlen);

		if (!getnameinfo(&p_self->super.remote_addr, p_self->super.remote_len, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST))
		{
			logit(LOG_INFO, "%s, connecting to %s(%s)", msg, p_self->super.p_remote_host_name, host);
		}

		if (0 != connect(p_self->super.socket, &p_self->super.remote_addr, p_self->super.remote_len))
		{
			int code = os_get_socket_error();

			logit(LOG_WARNING, MODULE_TAG "Failed connecting to remote server: %s", strerror(code));
			rc = RC_IP_CONNECT_FAILED;
			break;
		}
	}
	while (0);

	if (rc != RC_OK)
	{
		tcp_shutdown(p_self);
		return rc;
	}

	return RC_OK;
}
/*
  Disconnect and some other clean up.
*/
RC_TYPE tcp_shutdown(TCP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_OK;
	}

	p_self->initialized = FALSE;

	return ip_shutdown(&p_self->super);
}


/* send data*/
RC_TYPE tcp_send(TCP_SOCKET *p_self, const char *p_buf, int len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_TCP_OBJECT_NOT_INITIALIZED;
	}

	return ip_send(&p_self->super, p_buf, len);
}

/* receive data*/
RC_TYPE tcp_recv(TCP_SOCKET *p_self,char *p_buf, int max_recv_len, int *p_recv_len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_TCP_OBJECT_NOT_INITIALIZED;
	}
	return ip_recv(&p_self->super, p_buf, max_recv_len, p_recv_len);
}


/* Accessors*/
RC_TYPE tcp_set_port(TCP_SOCKET *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_set_port(&p_self->super, p);
}

RC_TYPE tcp_set_remote_name(TCP_SOCKET *p_self, const char* p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_set_remote_name(&p_self->super, p);
}

RC_TYPE tcp_set_remote_timeout(TCP_SOCKET *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_set_remote_timeout(&p_self->super, p);
}

RC_TYPE tcp_set_bind_iface(TCP_SOCKET *p_self, char *ifname)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_set_bind_iface(&p_self->super, ifname);
}

RC_TYPE tcp_get_port(TCP_SOCKET *p_self, int *p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_get_port(&p_self->super, p);
}

RC_TYPE tcp_get_remote_name(TCP_SOCKET *p_self, const char **p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_get_remote_name(&p_self->super, p);
}

RC_TYPE tcp_get_remote_timeout(TCP_SOCKET *p_self, int *p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_get_remote_timeout(&p_self->super, p);
}

RC_TYPE tcp_get_bind_iface(TCP_SOCKET *p_self, char **ifname)
{
	if (p_self == NULL || ifname == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return ip_get_bind_iface(&p_self->super, ifname);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
