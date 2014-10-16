/* Interface for HTTP functions
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
 * Boston, MA 02110-1301, USA.
 */

#include <string.h>

#include "ssl.h"
#include "http.h"
#include "error.h"

int http_construct(http_t *client)
{
	ASSERT(client);

	DO(tcp_construct(&client->tcp));

	memset((char *)client + sizeof(client->tcp), 0, sizeof(*client) - sizeof(client->tcp));
	client->initialized = 0;

	return 0;
}

/* Resource free. */
int http_destruct(http_t *client, int num)
{
	int i = 0, rv = 0;

	while (i < num)
		rv = tcp_destruct(&client[i++].tcp);

	return rv;
}

/* Set default TCP specififc params */
static int local_set_params(http_t *client)
{
	int timeout = 0;
	int port;

	http_get_remote_timeout(client, &timeout);
	if (timeout == 0)
		http_set_remote_timeout(client, HTTP_DEFAULT_TIMEOUT);

	http_get_port(client, &port);
	if (port == 0)
		http_set_port(client, HTTP_DEFAULT_PORT);

	return 0;
}

/* Sets up the object. */
int http_init(http_t *client, char *msg)
{
	int rc = 0;

	if (client->ssl_enabled)
		client->tcp.ip.port = HTTPS_DEFAULT_PORT;

	do {
		TRY(local_set_params(client));
		TRY(tcp_init(&client->tcp, msg, client->verbose));
		if (client->ssl_enabled)
			TRY(ssl_init(client, msg));
	}
	while (0);

	if (rc) {
		http_exit(client);
		return rc;
	}

	client->initialized = 1;

	return 0;
}

/* Disconnect and some other clean up. */
int http_exit(http_t *client)
{
	ASSERT(client);

	if (!client->initialized)
		return 0;

	client->initialized = 0;
	if (client->ssl_enabled)
		ssl_exit(client);

	return tcp_exit(&client->tcp);
}

static void http_response_parse(http_trans_t *trans)
{
	char *body;
	char *rsp = trans->p_rsp_body = trans->p_rsp;
	int status = trans->status = 0;
	const char sep[] = "\r\n\r\n";

	memset(trans->status_desc, 0, sizeof(trans->status_desc));

	if (rsp != NULL && (body = strstr(rsp, sep)) != NULL) {
		body += strlen(sep);
		trans->p_rsp_body = body;
	}

	/* %*c         : HTTP/1.0, 1.1 etc, discard read value
	 * %4d         : HTTP status code, e.g. 200
	 * %255[^\r\n] : HTTP status text, e.g. OK -- Reads max 255 bytes, including \0, not \r or \n
	 */
	if (sscanf(trans->p_rsp, "HTTP/1.%*c %4d %255[^\r\n]", &status, trans->status_desc) == 2)
		trans->status = status;
}

/* Send req and get response */
int http_transaction(http_t *client, http_trans_t *trans)
{
	int rc = 0;

	ASSERT(client);
	ASSERT(trans);

	if (!client->initialized)
		return RC_HTTP_OBJECT_NOT_INITIALIZED;

	trans->rsp_len = 0;
	do {
#ifdef ENABLE_SSL
		if (client->ssl_enabled) {
			TRY(ssl_send(client, trans->p_req, trans->req_len));
			TRY(ssl_recv(client, trans->p_rsp, trans->max_rsp_len, &trans->rsp_len));
		}
		else
#endif
		{
			TRY(tcp_send(&client->tcp, trans->p_req, trans->req_len));
			TRY(tcp_recv(&client->tcp, trans->p_rsp, trans->max_rsp_len, &trans->rsp_len));
		}
	}
	while (0);

	trans->p_rsp[trans->rsp_len] = 0;
	http_response_parse(trans);

	return rc;
}

/*
 * Validate HTTP status code
 */
int http_status_valid(int status)
{
	if (status == 200)
		return 0;

	if (status >= 500 && status < 600)
		return RC_DYNDNS_RSP_RETRY_LATER;

	return RC_DYNDNS_RSP_NOTOK;
}

int http_set_port(http_t *client, int port)
{
	ASSERT(client);
	return tcp_set_port(&client->tcp, port);
}

int http_get_port(http_t *client, int *port)
{
	ASSERT(client);
	return tcp_get_port(&client->tcp, port);
}


int http_set_remote_name(http_t *client, const char *name)
{
	ASSERT(client);
	return tcp_set_remote_name(&client->tcp, name);
}

int http_get_remote_name(http_t *client, const char **name)
{
	ASSERT(client);
	return tcp_get_remote_name(&client->tcp, name);
}

int http_set_remote_timeout(http_t *client, int timeout)
{
	ASSERT(client);
	return tcp_set_remote_timeout(&client->tcp, timeout);
}

int http_get_remote_timeout(http_t *client, int *timeout)
{
	ASSERT(client);
	return tcp_get_remote_timeout(&client->tcp, timeout);
}

int http_set_bind_iface(http_t *client, char *ifname)
{
	ASSERT(client);
	return tcp_set_bind_iface(&client->tcp, ifname);
}

int http_get_bind_iface(http_t *client, char **ifname)
{
	ASSERT(client);
	ASSERT(ifname);
	return tcp_get_bind_iface(&client->tcp, ifname);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
