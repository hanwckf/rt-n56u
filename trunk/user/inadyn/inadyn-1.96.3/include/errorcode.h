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

/* error code definitions */
#ifndef _ERRORCODE_INCLUDED
#define _ERRORCODE_INCLUDED

typedef enum 
{
	RC_OK = 0,
	RC_ERROR = 1,
	RC_INVALID_POINTER = 2,
	RC_OUT_OF_MEMORY = 3,
	RC_OUT_BUFFER_OVERFLOW = 4,
    
	RC_IP_SOCKET_CREATE_ERROR = 0x10,
	RC_IP_BAD_PARAMETER = 0x11,
	RC_IP_INVALID_REMOTE_ADDR = 0x12,
	RC_IP_CONNECT_FAILED = 0x13,
	RC_IP_SEND_ERROR = 0x14,
	RC_IP_RECV_ERROR = 0x15,
	RC_IP_OBJECT_NOT_INITIALIZED = 0x16,
	RC_IP_OS_SOCKET_INIT_FAILED = 0x17,

	RC_TCP_OBJECT_NOT_INITIALIZED = 0x20,

	RC_HTTP_OBJECT_NOT_INITIALIZED = 0x30,
	RC_HTTP_BAD_PARAMETER = 0x31,

	RC_DYNDNS_BUFFER_TOO_SMALL = 0x40,
	RC_DYNDNS_INVALID_IP_ADDR_IN_HTTP_RESPONSE = 0x41,
	RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER = 0x42,
	RC_DYNDNS_TOO_MANY_ALIASES = 0x43,
	RC_DYNDNS_INVALID_OPTION = 0x44,
	RC_DYNDNS_INVALID_OR_MISSING_PARAMETERS = 0x45,
	RC_DYNDNS_UNRESOLVED_ALIAS = 0x46,
    RC_DYNDNS_INVALID_DNS_SYSTEM_DEFAULT = 0x47,
    RC_DYNDNS_RSP_NOTOK = 0x48,

	RC_CMD_PARSER_INVALID_OPTION = 0x50,
	RC_CMD_PARSER_INVALID_OPTION_ARGUMENT = 0x51,

	RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER = 0x60,
    RC_OS_INVALID_IP_ADDRESS = 0x61,
    RC_OS_FORK_FAILURE = 0x62,
	RC_OS_CHANGE_PERSONA_FAILURE = 0x63,
    
    RC_FILE_IO_OPEN_ERROR = 0x70,
    RC_FILE_IO_READ_ERROR = 0x71,
	RC_FILE_IO_OUT_OF_BUFFER = 0x72
    

} RC_TYPE;

const char* errorcode_get_name(RC_TYPE rc);

#endif /* ERRORCODE_INCLUDED */
