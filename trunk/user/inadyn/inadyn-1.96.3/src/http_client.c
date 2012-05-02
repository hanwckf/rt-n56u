/*
Copyright (C) 2003-2004 Narcis Ilisei

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <string.h>
#include "http_client.h"
#include "errorcode.h"


#define super_construct(p) tcp_construct(p)
#define super_destruct(p)  tcp_destruct(p)
#define super_init(p)	   tcp_initialize(p)
#define super_shutdown(p)  tcp_shutdown(p)


/*public functions*/

/*
	 basic resource allocations for the tcp object
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

	/*free*/
	while(i < num)
	{
		rv = super_destruct(&p_self[i++].super);
	}
	return rv;
}


static RC_TYPE local_set_params(HTTP_CLIENT *p_self)
{
	{
		int timeout;
		/*set default TCP specififc params*/
		http_client_get_remote_timeout(p_self, &timeout);

		if (timeout == 0)
		{
			http_client_set_remote_timeout(p_self, HTTP_DEFAULT_TIMEOUT);
		}
	}

	{		
		int port;
		http_client_get_port(p_self, &port);
		if ( port == 0)
		{
			http_client_set_port(p_self, HTTP_DEFAULT_PORT);
		}
	}
	return RC_OK;
}

/* 
	Sets up the object.

	- ...
*/
RC_TYPE http_client_init(HTTP_CLIENT *p_self)
{
	RC_TYPE rc;
	do
	{
		/*set local params*/
		rc = local_set_params(p_self);
		if (rc != RC_OK)
		{
			break;
		}


		/*call super*/
		rc = super_init(&p_self->super);
		if (rc != RC_OK)
		{
			break;
		}

		/*local init*/

	}
	while(0);

	if (rc != RC_OK)
	{
		http_client_shutdown(p_self);		
	}
	else
	{
		p_self->initialized = TRUE;
	}
	
	return rc;
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

/* Send req and get response */
RC_TYPE http_client_transaction(HTTP_CLIENT *p_self, HTTP_TRANSACTION *p_tr )
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
	while(0);

	return rc;
}

/* Accessors */

RC_TYPE http_client_set_port(HTTP_CLIENT *p_self, int p)
{
	return tcp_set_port(&p_self->super, p);
}

RC_TYPE http_client_set_remote_name(HTTP_CLIENT *p_self, const char* p)
{
	return tcp_set_remote_name(&p_self->super, p);
}

RC_TYPE http_client_set_remote_timeout(HTTP_CLIENT *p_self, int p)
{
	return tcp_set_remote_timeout(&p_self->super, p);
}

RC_TYPE http_client_get_port(HTTP_CLIENT *p_self, int *p)
{
	return tcp_get_port(&p_self->super, p);
}
RC_TYPE http_client_get_remote_name(HTTP_CLIENT *p_self, const char* *p)
{
	return tcp_get_remote_name(&p_self->super, p);
}
RC_TYPE http_client_get_remote_timeout(HTTP_CLIENT *p_self, int *p)
{
	return tcp_get_remote_timeout(&p_self->super, p);
}

