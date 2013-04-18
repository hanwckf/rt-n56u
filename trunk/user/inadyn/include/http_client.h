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

/*interface for http_client */

#ifndef _HTTP_CLIENT_H_INCLUDED
#define _HTTP_CLIENT_H_INCLUDED

#include "os.h"
#include "errorcode.h"
#include "tcp.h"


/* SOME DEFAULT CONFIGURATIONS */
#define HTTP_DEFAULT_TIMEOUT	10000 /*ms*/
#define	HTTP_DEFAULT_PORT	80


typedef struct
{
	TCP_SOCKET super;
	BOOL initialized;
} HTTP_CLIENT;

typedef struct
{
	char *p_req;
	int req_len;

	char *p_rsp;
	int max_rsp_len;
	int rsp_len;

	char *p_rsp_body;

	int status;
	char status_desc[256];
} HTTP_TRANSACTION;


/*public functions*/

/*
	basic resource allocations for the tcp object
*/
RC_TYPE http_client_construct(HTTP_CLIENT *p_self);

/*
	Resource free.
*/
RC_TYPE http_client_destruct(HTTP_CLIENT *p_self, int num);

/*
	Sets up the object.

	- ...
*/
RC_TYPE http_client_init(HTTP_CLIENT *p_self, char *msg);

/*
	Disconnect and some other clean up.
*/
RC_TYPE http_client_shutdown(HTTP_CLIENT *p_self);

/* Send req and get response */
RC_TYPE http_client_transaction(HTTP_CLIENT *p_self, HTTP_TRANSACTION *p_tr );

/* Accessors */
RC_TYPE http_client_set_port(HTTP_CLIENT *p_self, int p);
RC_TYPE http_client_set_remote_name(HTTP_CLIENT *p_self, const char* p);
RC_TYPE http_client_set_remote_addr(HTTP_CLIENT *p_self, const char* p);
RC_TYPE http_client_set_remote_timeout(HTTP_CLIENT *p_self, int t);
RC_TYPE http_client_set_bind_iface(HTTP_CLIENT *p_self, char *ifname);

RC_TYPE http_client_get_port(HTTP_CLIENT *p_self, int *p_port);
RC_TYPE http_client_get_remote_name(HTTP_CLIENT *p_self, const char* *p);
RC_TYPE http_client_get_remote_timeout(HTTP_CLIENT *p_self, int *p);
RC_TYPE http_client_get_bind_iface(HTTP_CLIENT *p_self, char **ifname);

#endif /*_HTTP_CLIENT_H_INCLUDED*/
