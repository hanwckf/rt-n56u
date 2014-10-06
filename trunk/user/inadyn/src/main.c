/* Inadyn is a small and simple dynamic DNS (DDNS) client
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

#include <stdlib.h>

#include "debug.h"
#include "ddns.h"
#include "error.h"

/**
   basic resource allocations for the dyn_dns object
*/
static int alloc_context(ddns_t **pctx)
{
	int rc = 0;
	ddns_t *ctx;
	int http_to_dyndns_constructed = 0;
	int http_to_ip_constructed = 0;

	if (!pctx)
		return RC_INVALID_POINTER;

	*pctx = (ddns_t *)malloc(sizeof(ddns_t));
	if (!*pctx)
		return RC_OUT_OF_MEMORY;

	do {
		int i;

		ctx = *pctx;
		memset(ctx, 0, sizeof(ddns_t));

		/* Alloc space for http_to_ip_server data */
		ctx->work_buflen = DYNDNS_HTTP_RESPONSE_BUFFER_SIZE;
		ctx->work_buf = (char *)malloc(ctx->work_buflen);
		if (!ctx->work_buf) {
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		/* Alloc space for request data */
		ctx->request_buflen = DYNDNS_HTTP_REQUEST_BUFFER_SIZE;
		ctx->request_buf = (char *)malloc(ctx->request_buflen);
		if (!ctx->request_buf) {
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER) {
			if (http_construct(&ctx->http_to_ip_server[i++])) {
				rc = RC_OUT_OF_MEMORY;
				break;
			}
		}
		http_to_ip_constructed = 1;

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER) {
			if (http_construct(&ctx->http_to_dyndns[i++])) {
				rc = RC_OUT_OF_MEMORY;
				break;
			}
		}
		http_to_dyndns_constructed = 1;

		ctx->cmd = NO_CMD;
		ctx->startup_delay_sec = DYNDNS_DEFAULT_STARTUP_SLEEP;
		ctx->normal_update_period_sec = DYNDNS_DEFAULT_SLEEP;
		ctx->sleep_sec = DYNDNS_DEFAULT_SLEEP;
		ctx->total_iterations = DYNDNS_DEFAULT_ITERATIONS;
		ctx->cmd_check_period = DYNDNS_DEFAULT_CMD_CHECK_PERIOD;
		ctx->force_addr_update = 0;

		i = 0;
		while (i < DYNDNS_MAX_SERVER_NUMBER)
			ctx->info[i++].creds.encoded_password = NULL;

		ctx->initialized = 0;
	}
	while (0);

	if (rc) {

		if (ctx->work_buf)
			free(ctx->work_buf);

		if (ctx->request_buf)
			free(ctx->request_buf);

		if (http_to_dyndns_constructed)
			http_destruct(ctx->http_to_dyndns, DYNDNS_MAX_SERVER_NUMBER);

		if (http_to_ip_constructed)
			http_destruct(ctx->http_to_ip_server, DYNDNS_MAX_SERVER_NUMBER);

		free(ctx);
		*pctx = NULL;
	}

	return 0;
}

static void free_context(ddns_t *ctx)
{
	int i;

	if (!ctx)
		return;

	http_destruct(ctx->http_to_ip_server, DYNDNS_MAX_SERVER_NUMBER);
	http_destruct(ctx->http_to_dyndns, DYNDNS_MAX_SERVER_NUMBER);

	if (ctx->work_buf) {
		free(ctx->work_buf);
		ctx->work_buf = NULL;
	}

	if (ctx->request_buf) {
		free(ctx->request_buf);
		ctx->request_buf = NULL;
	}

	for (i = 0; i < DYNDNS_MAX_SERVER_NUMBER; i++) {
		ddns_info_t *info = &ctx->info[i];

		if (info->creds.encoded_password) {
			free(info->creds.encoded_password);
			info->creds.encoded_password = NULL;
		}
	}

	if (ctx->cfgfile) {
		free(ctx->cfgfile);
		ctx->cfgfile = NULL;
	}

	if (ctx->external_command) {
		free(ctx->external_command);
		ctx->external_command = NULL;
	}

	if (ctx->bind_interface) {
		free(ctx->bind_interface);
		ctx->bind_interface = NULL;
	}

	if (ctx->check_interface) {
		free(ctx->check_interface);
		ctx->check_interface = NULL;
	}

	free(ctx);
}

int main(int argc, char *argv[])
{
	int rc = 0, restart;
	ddns_t *ctx = NULL;

#ifdef ENABLE_SSL
	SSL_library_init();
	SSL_load_error_strings();
#endif

	do {
		restart = 0;

		rc = alloc_context(&ctx);
		if (rc == RC_OK) {
			DO(os_install_signal_handler(ctx));

			rc = ddns_main_loop(ctx, argc, argv);
			if (rc == RC_RESTART)
				restart = 1;

			free_context(ctx);
		}
	} while (restart);

	os_close_dbg_output();

#ifdef ENABLE_SSL
	ERR_free_strings();
#endif

	return rc;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
