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

#ifndef _IP_H_INCLUDED
#define _IP_H_INCLUDED

#include "os.h"
#include "errorcode.h"


/* SOME DEFAULT CONFIGURATIONS */
#define IP_DEFAULT_TIMEOUT	20000 /*ms*/
#define IP_SOCKET_MAX_PORT 65535
#define IP_DEFAULT_READ_CHUNK_SIZE	100

/* typedefs */
typedef enum
{
	TYPE_TCP,
	TYPE_UDP
} IP_TYPES;

typedef struct 
{
	BOOL initialized;
	int type;
	SOCKET socket; 
	struct sockaddr_in remote_addr;
	const char *p_remote_host_name;  

	unsigned short port;
	int timeout;
} IP_SOCKET;


/*public functions*/

/*
	 basic resource allocations for the  object
*/
RC_TYPE ip_construct(IP_SOCKET *p_self);

/*
	Resource free .
*/	
RC_TYPE ip_destruct(IP_SOCKET *p_self);

/* 
	Sets up the object.

	- ...
*/
RC_TYPE ip_initialize(IP_SOCKET *p_self);

/* 
	Disconnect and some other clean up.
*/
RC_TYPE ip_shutdown(IP_SOCKET *p_self);

/* send data*/
RC_TYPE ip_send(IP_SOCKET *p_self, const char *p_buf, int len);

/* receive data*/
RC_TYPE ip_recv(IP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len);


/*Accessors */
RC_TYPE ip_set_port(IP_SOCKET *p_self, int p);
RC_TYPE ip_set_remote_name(IP_SOCKET *p_self, const char* p);
RC_TYPE ip_set_remote_timeout(IP_SOCKET *p_self, int t);

RC_TYPE ip_get_port(IP_SOCKET *p_self, int *p_port);
RC_TYPE ip_get_remote_name(IP_SOCKET *p_self, const char* *p);
RC_TYPE ip_get_remote_timeout(IP_SOCKET *p_self, int *p);

#endif /*_IP_H_INCLUDED*/
