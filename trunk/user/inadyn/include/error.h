/* Error code definitions
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

#ifndef INADYN_ERROR_H_
#define INADYN_ERROR_H_

#define RC_OK                                       0
#define RC_ERROR                                    1
#define RC_INVALID_POINTER                          2
#define RC_OUT_OF_MEMORY                            3
#define RC_OUT_BUFFER_OVERFLOW                      4

#define RC_IP_SOCKET_CREATE_ERROR                   10
#define RC_IP_BAD_PARAMETER                         11
#define RC_IP_INVALID_REMOTE_ADDR                   12
#define RC_IP_CONNECT_FAILED                        13
#define RC_IP_SEND_ERROR                            14
#define RC_IP_RECV_ERROR                            15
#define RC_IP_OBJECT_NOT_INITIALIZED                16
#define RC_IP_OS_SOCKET_INIT_FAILED                 17
#define RC_IP_SOCKET_BIND_ERROR                     18

#define RC_TCP_OBJECT_NOT_INITIALIZED               20

#define RC_HTTP_OBJECT_NOT_INITIALIZED              30
#define RC_HTTP_BAD_PARAMETER                       31
#define RC_HTTPS_OUT_OF_MEMORY                      32
#define RC_HTTPS_FAILED_CONNECT                     33
#define RC_HTTPS_FAILED_GETTING_CERT                34
#define RC_HTTPS_NO_SSL_SUPPORT                     35
#define RC_HTTPS_SEND_ERROR                         36
#define RC_HTTPS_RECV_ERROR                         37
#define RC_HTTPS_SNI_ERROR                          38

#define RC_DYNDNS_BUFFER_TOO_SMALL                  40
#define RC_DYNDNS_INVALID_IP_ADDR_IN_HTTP_RESPONSE  41
#define RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER        42
#define RC_DYNDNS_TOO_MANY_ALIASES                  43
#define RC_DYNDNS_INVALID_OPTION                    44
#define RC_DYNDNS_INVALID_OR_MISSING_PARAMETERS     45
#define RC_DYNDNS_UNRESOLVED_ALIAS                  46
#define RC_DYNDNS_INVALID_DNS_SYSTEM_DEFAULT        47
#define RC_DYNDNS_RSP_NOTOK                         48
#define RC_DYNDNS_RSP_RETRY_LATER                   49

#define RC_CMD_PARSER_INVALID_OPTION                50
#define RC_CMD_PARSER_INVALID_OPTION_ARGUMENT       51

#define RC_OS_ERROR_INSTALLING_SIGNAL_HANDLER       60
#define RC_OS_INVALID_IP_ADDRESS                    61
#define RC_OS_FORK_FAILURE                          62
#define RC_OS_CHANGE_PERSONA_FAILURE                63
#define RC_OS_INVALID_UID                           64
#define RC_OS_INVALID_GID                           65
#define RC_OS_INSTALL_SIGHANDLER_FAILED             66

#define RC_FILE_IO_OPEN_ERROR                       70
#define RC_FILE_IO_READ_ERROR                       71
#define RC_FILE_IO_OUT_OF_BUFFER                    72
#define RC_FILE_IO_ACCESS_ERROR                     73

#define RC_RESTART                                  255

#define DO(fn)       { int rc = fn; if (rc) return rc; }
#define TRY(fn)      {     rc = fn; if (rc) break; }
#define ASSERT(cond) { if (!cond) return RC_INVALID_POINTER; }
#define UNUSED(var)  (var)__attribute__((unused))

const char *errorcode_get_name(int rc);

#endif /* INADYN_ERROR_H_ */

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
