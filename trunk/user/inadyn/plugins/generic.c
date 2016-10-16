/* Generic DDNS plugin
 *
 * Copyright (C) 2003-2004  Narcis Ilisei <inarcis2002@hotpop.com>
 * Copyright (C) 2006       Steve Horbachuk
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

#include "plugin.h"

/*
 * Generic update format for sites that perform the update using:
 *
 *	http://some.address.domain/somesubdir?some_param_name=ALIAS
 *
 * With the standard http stuff and basic base64 encoded auth.
 * The parameter here is the entire request, except the the alias.
 */
#define GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST				\
	"GET %s%s "							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: " AGENT_NAME " " SUPPORT_ADDR "\r\n\r\n"

static int request  (ddns_t       *ctx,   ddns_info_t *info, ddns_alias_t *alias);
static int response (http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t generic = {
	.name         = "custom@http_srv_basic_auth",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,

	.server_name  = "",
	.server_url   = ""
};

/* Compatiblity entry, new canonical is @http_srv */
static ddns_system_t compat = {
	.name         = "custom@http_svr_basic_auth",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,

	.server_name  = "",
	.server_url   = ""
};

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char *arg = alias->name; /* Backwards compat, default to append hostname */

	if (info->append_myip)	 /* New, append client's IP address instead. */
		arg = alias->address;

	return snprintf(ctx->request_buf, ctx->request_buflen,
			GENERIC_BASIC_AUTH_UPDATE_IP_REQUEST,
			info->server_url, arg,
			info->server_name.name,
			info->creds.encoded_password);
}

/*
 * generic response validator
 * parses a given string. If found is ok, returns OK (0)
 *
 * Example: 'SUCCESS CODE='
 */
static int response(http_trans_t *trans, ddns_info_t *UNUSED(info), ddns_alias_t *UNUSED(alias))
{
	char *resp = trans->p_rsp_body;

	DO(http_status_valid(trans->status));

	if (strstr(resp, "OK") || strstr(resp, "good") || strstr(resp, "true") || strcasestr(resp, "updated"))
		return RC_OK;

	return RC_DYNDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&generic);
	plugin_register(&compat);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&generic);
	plugin_unregister(&compat);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
