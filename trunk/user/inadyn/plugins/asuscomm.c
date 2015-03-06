/* Plugin for asuscomm.com DDNS
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

#include "md5.h"
#include "base64.h"
#include "plugin.h"

#include <ctype.h>

#include <nvram_linux.h>

#define ASUSDDNS_IP_HTTP_REQUEST					\
	"GET %s?"							\
	"hostname=%s&"							\
	"myip=%s "							\
	"HTTP/1.0\r\n"							\
	"Authorization: Basic %s\r\n"					\
	"Host: %s\r\n"							\
	"User-Agent: " AGENT_NAME " " SUPPORT_ADDR "\r\n\r\n"

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias);
static int response_update(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);
static int response_register(http_trans_t *trans, ddns_info_t *info, ddns_alias_t *alias);

static ddns_system_t asus_update = {
	.name         = "update@asus.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_update,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,

	.server_name  = "ns1.asuscomm.com",
	.server_url   = "/ddns/update.jsp"
};

static ddns_system_t asus_register = {
	.name         = "register@asus.com",

	.request      = (req_fn_t)request,
	.response     = (rsp_fn_t)response_register,

	.checkip_name = DYNDNS_MY_IP_SERVER,
	.checkip_url  = DYNDNS_MY_CHECKIP_URL,

	.server_name  = "ns1.asuscomm.com",
	.server_url   = "/ddns/register.jsp"
};

#define MD5_DIGEST_BYTES 16
static void
hmac_md5( const unsigned char *input, size_t ilen, const unsigned char *key, size_t klen, unsigned char output[MD5_DIGEST_BYTES] )
{
	int i;
	md5_context ctx;
	unsigned char k_ipad[64], k_opad[64], tk[MD5_DIGEST_BYTES];

	/* if key is longer than 64 bytes reset it to key=MD5(key) */
	if (klen > 64) {
		md5(key, klen, tk);
		key = tk;
		klen = MD5_DIGEST_BYTES;
	}

	/* start out by storing key in pads */
	memset(k_ipad, 0, sizeof(k_ipad));
	memset(k_opad, 0, sizeof(k_opad));
	memcpy(k_ipad, key, klen);
	memcpy(k_opad, key, klen);

	/*xor key with ipad and opad values */
	for (i = 0; i < 64; i++) {
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}

	/* inner MD5 */
	md5_starts( &ctx );
	md5_update( &ctx, k_ipad, 64 );
	md5_update( &ctx, input, ilen );
	md5_finish( &ctx, output );

	/* outter MD5 */
	md5_starts( &ctx );
	md5_update( &ctx, k_opad, 64 );
	md5_update( &ctx, output, MD5_DIGEST_BYTES );
	md5_finish( &ctx, output );

	memset( &ctx, 0, sizeof( md5_context ) );
}

static int request(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias)
{
	unsigned char digest[MD5_DIGEST_BYTES];
	char auth[6*2+1+MD5_DIGEST_BYTES*2+1];
	char *p_tmp, *p_auth = auth;
	char *p_b64_buff = NULL;
	size_t dlen = 0;
	int i;

	/* prepare username (MAC) */
	p_tmp = info->creds.username;
	for (i = 0; i < 6*2; i++) {
		while (*p_tmp && !isxdigit(*p_tmp))
			p_tmp++;
		*p_auth++ = *p_tmp ? toupper(*p_tmp++) : '0';
	}

	/* split username and password */
	*p_auth++ = ':';

	/* prepare password, reuse request_buf */
	snprintf(ctx->request_buf, ctx->request_buflen, "%s%s",
		alias->name,
		alias->address);
	hmac_md5(ctx->request_buf, strlen(ctx->request_buf),
		info->creds.password, strlen(info->creds.password), digest);
	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		p_auth += sprintf(p_auth, "%02X", digest[i]);

	/*encode*/
	base64_encode(NULL, &dlen, auth, strlen(auth));
	p_b64_buff = (char *) malloc(dlen);
	if (p_b64_buff)
		base64_encode(p_b64_buff, &dlen, auth, strlen(auth));

	if (info->creds.encoded_password)
		free(info->creds.encoded_password);

	info->creds.encoded_password = p_b64_buff;
	info->creds.encoded = (p_b64_buff != NULL) ? 1 : 0;
	info->creds.size = strlen(info->creds.encoded_password);

	return snprintf(ctx->request_buf, ctx->request_buflen,
			ASUSDDNS_IP_HTTP_REQUEST,
			info->server_url,
			alias->name,
			alias->address,
			info->creds.encoded_password,
			info->server_name.name);
}

static int response_update(http_trans_t *trans, ddns_info_t *UNUSED(info), ddns_alias_t *UNUSED(alias))
{
	char *p, *p_rsp;
	char domain[256] = {0};

	p_rsp = trans->p_rsp_body;

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	switch (trans->status) {
		case 200:		/* update success */
		case 220:		/* update same domain success -- unused?? */
			return RC_OK;
		case 203:		/* update failed */
			logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
			return RC_DYNDNS_RSP_NOTOK;
		case 233:		/* update failed */
			logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
			return RC_DYNDNS_RSP_NOTOK;
		case 297:		/* invalid hostname */
		case 298:		/* invalid domain name */
		case 299:		/* invalid ip format */
		case 401:		/* authentication failure */
		case 407:		/* proxy authentication required */
			return RC_DYNDNS_RSP_NOTOK;
	}

	if (trans->status >= 500 && trans->status < 600)
		return RC_DYNDNS_RSP_RETRY_LATER;

	return RC_DYNDNS_RSP_NOTOK;
}

static int response_register(http_trans_t *trans, ddns_info_t *UNUSED(info), ddns_alias_t *UNUSED(alias))
{
	char *p, *p_rsp, *ret_key;
	char ret_buf[64], domain[256] = {0};

	ret_key = "ddns_return_code";

	p_rsp = trans->p_rsp_body;

	if ((p = strchr(p_rsp, '|')) && (p = strchr(++p, '|')))
		sscanf(p, "|%255[^|\r\n]", domain);

	if (trans->status >= 500 && trans->status < 600) {
		nvram_set_temp(ret_key, "unknown_error");
		return RC_DYNDNS_RSP_RETRY_LATER;
	}

	snprintf(ret_buf, sizeof(ret_buf), "%s,%d", "register", trans->status);
	nvram_set_temp(ret_key, ret_buf);

	switch (trans->status) {
	case 200:		/* registration success */
	case 220:		/* registration same domain success*/
		return RC_OK;
	case 203:		/* registration failed */
		logit(LOG_WARNING, "Domain already in use, suggested domain '%s'", domain);
		return RC_DYNDNS_RSP_NOTOK;
	case 230:		/* registration new domain success */
		logit(LOG_WARNING, "Registration success, previous domain '%s'", domain);
		return RC_OK;
	case 233:		/* registration failed */
		logit(LOG_WARNING, "Domain already in use, current domain '%s'", domain);
		return RC_DYNDNS_RSP_NOTOK;
	case 297:		/* invalid hostname */
	case 298:		/* invalid domain name */
	case 299:		/* invalid ip format */
	case 401:		/* authentication failure */
	case 407:		/* proxy authentication required */
		return RC_DYNDNS_RSP_NOTOK;
	}

	if (trans->status < 500)		/* shutdown */
		nvram_set_temp(ret_key, "time_out");

	return RC_DYNDNS_RSP_NOTOK;
}

PLUGIN_INIT(plugin_init)
{
	plugin_register(&asus_update);
	plugin_register(&asus_register);
}

PLUGIN_EXIT(plugin_exit)
{
	plugin_unregister(&asus_update);
	plugin_unregister(&asus_register);
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
