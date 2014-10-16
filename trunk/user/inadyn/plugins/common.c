/* Common plugin methods, built around DynDNS standard HTTP API
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
 * dyndns.org specific update address format
 *
 * Also applies to other dyndns2 api compatible services, like:
 * DNS-O-Matic, DynSIP, no-ip, 3322, HE and nsupdate.info.
 */
#define DYNDNS_UPDATE_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	"myip=%s"							\
	"%s "      							\
	"HTTP/1.0\r\n"							\
	"Host: %s\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"User-Agent: " AGENT_NAME " " SUPPORT_ADDR "\r\n\r\n"

/*
 * DynDNS request composer -- common to many other DDNS providers as well
 */
int common_request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	char wildcard[20] = "";

	if (info->wildcard)
		strcpy(wildcard, "&wildcard=ON");

	return snprintf(ctx->request_buf, ctx->request_buflen,
			DYNDNS_UPDATE_IP_HTTP_REQUEST,
			info->server_url,
			alias->name,
			alias->address,
			wildcard,
			info->server_name.name,
			info->creds.encoded_password);
}

/*
 * DynDNS response validator -- common to many other DDNS providers as well
 *  'good' or 'nochg' are the good answers,
 */
int common_response(http_trans_t *trans, ddns_info_t *UNUSED(info), ddns_alias_t *UNUSED(alias))
{
	char *body = trans->p_rsp_body;

	DO(http_status_valid(trans->status));

	if (strstr(body, "good") != NULL || strstr(body, "nochg"))
		return RC_OK;

	if (strstr(body, "dnserr") != NULL || strstr(body, "911"))
		return RC_DYNDNS_RSP_RETRY_LATER;

	return RC_DYNDNS_RSP_NOTOK;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
