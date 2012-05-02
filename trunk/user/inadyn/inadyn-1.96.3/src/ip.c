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
#define MODULE_TAG "IP: "
#include <stdlib.h>
#include <string.h>

#include "debug_if.h"
#include "ip.h"



/*public functions*/


/*
	 basic resource allocations for the ip object
*/
RC_TYPE ip_construct(IP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	memset(p_self, 0, sizeof(IP_SOCKET));

	p_self->initialized = FALSE;
	p_self->socket = 0;
	memset( &p_self->remote_addr, 0,sizeof(p_self->remote_addr));
	p_self->timeout = IP_DEFAULT_TIMEOUT;
	
	return RC_OK;
}

/*
	Resource free.
*/	
RC_TYPE ip_destruct(IP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_OK;
	}

	if (p_self->initialized == TRUE)
	{
		ip_shutdown(p_self);
	}
		
	return RC_OK;
}



/* 
	Sets up the object.
	
	- ...
*/
RC_TYPE ip_initialize(IP_SOCKET *p_self)
{
	RC_TYPE rc = RC_OK; 

	if (p_self->initialized == TRUE)
	{
		return RC_OK;
	}

	do
	{
		rc = os_ip_support_startup();
		if (rc != RC_OK)
		{
			break;
		}

		/*remote addres */
		if (p_self->p_remote_host_name != NULL)
		{
            unsigned long addr = 0;
			HOSTENT* p_remotehost = (HOSTENT*) gethostbyname(p_self->p_remote_host_name);
			
			if (p_remotehost == NULL)
			{                
                rc = os_convert_ip_to_inet_addr(&addr, p_self->p_remote_host_name);
                if (rc != RC_OK)
                {
				    DBG_PRINTF((LOG_WARNING,MODULE_TAG "Error '0x%x' resolving host name '%s'\n", 
							    os_get_socket_error(),
							    p_self->p_remote_host_name));
				    rc = RC_IP_INVALID_REMOTE_ADDR;
				    break;
                }
			}
			
			p_self->remote_addr.sin_family = AF_INET;
			p_self->remote_addr.sin_port = htons(p_self->port); 
			p_self->remote_addr.sin_addr.s_addr = (addr == 0) ?
                    *((unsigned long *)p_remotehost->h_addr_list[0]) : addr; 
		}
	}
	while(0);

	if (rc != RC_OK)
	{
		ip_shutdown(p_self);		
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
RC_TYPE ip_shutdown(IP_SOCKET *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_OK;
	}

	if (p_self->socket)
	{
		closesocket(p_self->socket);
		p_self->socket = 0;
	}

	os_ip_support_cleanup();

	p_self->initialized = FALSE;
	return RC_OK;
}

RC_TYPE ip_send(IP_SOCKET *p_self, const char *p_buf, int len)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_IP_OBJECT_NOT_INITIALIZED;
	}
	
	if( send(p_self->socket, (char*) p_buf, len, 0) == SOCKET_ERROR )
	{
		DBG_PRINTF((LOG_WARNING,MODULE_TAG "Error 0x%x in send()\n", os_get_socket_error()));
		return RC_IP_SEND_ERROR;
	}
	return RC_OK;
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
RC_TYPE ip_recv(IP_SOCKET *p_self, char *p_buf, int max_recv_len, int *p_recv_len)
{
	RC_TYPE rc = RC_OK;
	int remaining_buf_len = max_recv_len;
	int total_recv_len = 0;
	int recv_len = 0;

	if (p_self == NULL || p_buf == NULL || p_recv_len == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (!p_self->initialized)
	{
		return RC_IP_OBJECT_NOT_INITIALIZED;
	}

	while (remaining_buf_len > 0)
	{
		int chunk_size = remaining_buf_len > IP_DEFAULT_READ_CHUNK_SIZE ? 
					IP_DEFAULT_READ_CHUNK_SIZE : remaining_buf_len;
		recv_len = recv(p_self->socket, p_buf + total_recv_len, chunk_size, 0);
		if (recv_len < 0)
		{
			
			{
				DBG_PRINTF((LOG_WARNING, MODULE_TAG "Error 0x%x in recv()\n", os_get_socket_error()));
				rc = RC_IP_RECV_ERROR;
			}
			break;
		}

		if (recv_len == 0)			
		{
			if (total_recv_len == 0)
			{
				rc = RC_IP_RECV_ERROR;				
			}
			break;
		}

		total_recv_len += recv_len;
		remaining_buf_len = max_recv_len - total_recv_len;
	}
	

	*p_recv_len = total_recv_len;
	return rc;
}


/*Accessors */

RC_TYPE ip_set_port(IP_SOCKET *p_self, int p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p < 0 || p > IP_SOCKET_MAX_PORT)
	{
		return RC_IP_BAD_PARAMETER;
	}
	p_self->port = p;
	return RC_OK;
}

RC_TYPE ip_set_remote_name(IP_SOCKET *p_self, const char* p)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->p_remote_host_name = p;
	return RC_OK;
}

RC_TYPE ip_set_remote_timeout(IP_SOCKET *p_self, int t)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->timeout = t;
	return RC_OK;
}

RC_TYPE ip_get_port(IP_SOCKET *p_self, int *p_port)
{
	if (p_self == NULL || p_port == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p_port = p_self->port;
	return RC_OK;
}

RC_TYPE ip_get_remote_name(IP_SOCKET *p_self, const char* *p)
{
	if (p_self == NULL || p == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p = p_self->p_remote_host_name;
	return RC_OK;
}

RC_TYPE ip_get_remote_timeout(IP_SOCKET *p_self, int *p)
{
	if (p_self == NULL || p == NULL)
	{
		return RC_INVALID_POINTER;
	}
	*p = p_self->timeout;
	return RC_OK;
}

