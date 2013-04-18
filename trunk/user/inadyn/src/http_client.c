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

#include <string.h>
#include "http_client.h"
#include "errorcode.h"

#define super_construct(p) tcp_construct(p)
#define super_destruct(p)  tcp_destruct(p)
#define super_init(p,msg)  tcp_initialize(p, msg)
#define super_shutdown(p)  tcp_shutdown(p)

/*
  basic resource allocations for the http object
*/
RC_TYPE http_client_construct(HTTP_CLIENT *p_self)
{
	RC_TYPE rc;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	rc = super_construct(&p_self->super);
	if (rc != RC_OK)
	{
		return rc;
	}

	/*init*/
	memset( (char*)p_self + sizeof(p_self->super), 0 , sizeof(*p_self) - sizeof(p_self->super));
	p_self->initialized = FALSE;

	return RC_OK;
}

/*
  Resource free.
*/
RC_TYPE http_client_destruct(HTTP_CLIENT *p_self, int num)
{
	int i = 0, rv = RC_OK;

	while (i < num)
	{
		rv = super_destruct(&p_self[i++].super);
	}

	return rv;
}


/* Set default TCP specififc params */
static RC_TYPE local_set_params(HTTP_CLIENT *p_self)
{
	int timeout, port;

	http_client_get_remote_timeout(p_self, &timeout);
	if (timeout == 0)
	{
		http_client_set_remote_timeout(p_self, HTTP_DEFAULT_TIMEOUT);
	}

	http_client_get_port(p_self, &port);
	if (port == 0)
	{
		http_client_set_port(p_self, HTTP_DEFAULT_PORT);
	}

	return RC_OK;
}

/*
  Sets up the object.

  - ...
*/
RC_TYPE http_client_init(HTTP_CLIENT *p_self, char *msg)
{
	RC_TYPE rc;

	do
	{
		rc = local_set_params(p_self);
		if (rc != RC_OK)
		{
			break;
		}

		rc = super_init(&p_self->super, msg);
		if (rc != RC_OK)
		{
			break;
		}
	}
	while (0);

	if (rc != RC_OK)
	{
		http_client_shutdown(p_self);
		return rc;
	}

	p_self->initialized = TRUE;

	return RC_OK;
}

/*
  Disconnect and some other clean up.
*/
RC_TYPE http_client_shutdown(HTTP_CLIENT *p_self)
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

	return super_shutdown(&p_self->super);
}

static void http_response_parse (HTTP_TRANSACTION *p_tr) {
	char *body;
	char *rsp = p_tr->p_rsp_body = p_tr->p_rsp;
	int status = p_tr->status = 0;
	memset(p_tr->status_desc, 0, sizeof(p_tr->status_desc));
	const char sep[] = "\r\n\r\n";
	if (rsp != NULL && (body = strstr(rsp, sep)) != NULL) {
		body += strlen(sep);
		p_tr->p_rsp_body = body;
	}
	if (sscanf(p_tr->p_rsp, "HTTP/1.%*c %d %255[^\r\n]", &status, p_tr->status_desc) == 2)
		p_tr->status = status;
}

/* Send req and get response */
RC_TYPE http_client_transaction(HTTP_CLIENT *p_self, HTTP_TRANSACTION *p_tr)
{
	RC_TYPE rc;
	if (p_self == NULL || p_tr == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_HTTP_OBJECT_NOT_INITIALIZED;
	}

	do
	{
		rc = tcp_send(&p_self->super, p_tr->p_req, p_tr->req_len);
		if (rc != RC_OK)
		{
			break;
		}

		rc = tcp_recv(&p_self->super, p_tr->p_rsp, p_tr->max_rsp_len, &p_tr->rsp_len);
	}
	while (0);
	
	http_response_parse(p_tr);

	return rc;
}

/* Accessors */

RC_TYPE http_client_set_port(HTTP_CLIENT *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_set_port(&p_self->super, p);
}

RC_TYPE http_client_set_remote_name(HTTP_CLIENT *p_self, const char* p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_set_remote_name(&p_self->super, p);
}

RC_TYPE http_client_set_remote_timeout(HTTP_CLIENT *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_set_remote_timeout(&p_self->super, p);
}

RC_TYPE http_client_set_bind_iface(HTTP_CLIENT *p_self, char *ifname)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_set_bind_iface(&p_self->super, ifname);
}

RC_TYPE http_client_get_port(HTTP_CLIENT *p_self, int *p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_get_port(&p_self->super, p);
}

RC_TYPE http_client_get_remote_name(HTTP_CLIENT *p_self, const char* *p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_get_remote_name(&p_self->super, p);
}

RC_TYPE http_client_get_remote_timeout(HTTP_CLIENT *p_self, int *p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_get_remote_timeout(&p_self->super, p);
}

RC_TYPE http_client_get_bind_iface(HTTP_CLIENT *p_self, char **ifname)
{
	if (p_self == NULL || ifname == NULL)
	{
		return RC_INVALID_POINTER;
	}

	return tcp_get_bind_iface(&p_self->super, ifname);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "ellemtel"
 *  c-basic-offset: 8
 * End:
 */
