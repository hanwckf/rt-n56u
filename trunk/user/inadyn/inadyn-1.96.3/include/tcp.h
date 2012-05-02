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


/* interface for tcp functions */

#ifndef _TCP_H_INCLUDED
#define _TCP_H_INCLUDED

#include "os.h"
#include "errorcode.h"
#include "ip.h"


/* SOME DEFAULT CONFIGURATIONS */
#define TCP_DEFAULT_TIMEOUT	20000 /*ms*/


typedef struct 
{
	IP_SOCKET super;
	BOOL initialized;
} TCP_SOCKET;

typedef struct
{
	TCP_SOCKET super;
} TCP_CLIENT_SOCKET;

/*public functions*/

/*
	 basic resource allocations for the tcp object
*/
RC_TYPE tcp_construct(TCP_SOCKET *p_self);

/*
	Resource free.
*/	
RC_TYPE tcp_destruct(TCP_SOCKET *p_self);

/* 
	Sets up the object.

	- ...
*/
RC_TYPE tcp_initialize(TCP_SOCKET *p_self);

/* 
	Disconnect and some other clean up.
*/
RC_TYPE tcp_shutdown(TCP_SOCKET *p_self);


/* send data*/
RC_TYPE tcp_send(TCP_SOCKET *p_self, const char *p_buf, int len);

/* receive data*/
RC_TYPE tcp_recv(TCP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len);

/* Accessors */

RC_TYPE tcp_set_port(TCP_SOCKET *p_self, int p);
RC_TYPE tcp_set_remote_name(TCP_SOCKET *p_self, const char* p);
RC_TYPE tcp_set_remote_timeout(TCP_SOCKET *p_self, int t);

RC_TYPE tcp_get_port(TCP_SOCKET *p_self, int *p_port);
RC_TYPE tcp_get_remote_name(TCP_SOCKET *p_self, const char* *p);
RC_TYPE tcp_get_remote_timeout(TCP_SOCKET *p_self, int *p);


#endif /*_TCP_H_INCLUDED*/
