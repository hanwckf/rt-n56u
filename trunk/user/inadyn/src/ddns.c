/* DDNS client updater main implementation file
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

#include <arpa/nameser.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "ddns.h"
#include "cache.h"
#include "debug.h"
#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "cmd.h"

/* Used to preserve values during reset at SIGHUP.  Time also initialized from cache file at startup. */
static int cached_num_iterations = 0;


static int wait_for_cmd(ddns_t *ctx)
{
	int counter;
	ddns_cmd_t old_cmd;

	if (!ctx)
		return RC_INVALID_POINTER;

	old_cmd = ctx->cmd;
	if (old_cmd != NO_CMD)
		return 0;

	counter = ctx->sleep_sec / ctx->cmd_check_period;
	while (counter--) {
		if (ctx->cmd != old_cmd)
			break;

		os_sleep_ms(ctx->cmd_check_period * 1000);
	}

	return 0;
}

/*
	Get the IP address from interface
*/
static int check_interface_address(ddns_t *ctx)
{
	struct ifreq ifr;
	in_addr_t addr;
	char *address;
	int i, j, sd, anychange = 0;

	if (ctx->dbg.level > 0)
		logit(LOG_INFO, "Checking for IP# change, querying interface %s", ctx->check_interface);

	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd < 0) {
		int code = os_get_socket_error();

		logit(LOG_WARNING, "Failed opening network socket: %s", strerror(code));
		return RC_IP_OS_SOCKET_INIT_FAILED;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ctx->check_interface);
	if (ioctl(sd, SIOCGIFADDR, &ifr) != -1) {
		addr = ntohl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
		address = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	} else {
		int code = os_get_socket_error();

		logit(LOG_ERR, "Failed reading IP address of interface %s: %s",
		      ctx->check_interface, strerror(code));
		close(sd);

		return RC_OS_INVALID_IP_ADDRESS;
	}
	close(sd);

	if (IN_ZERONET(addr)   || IN_LOOPBACK(addr)  ||
	    IN_LINKLOCAL(addr) || IN_MULTICAST(addr) || IN_EXPERIMENTAL(addr)) {
		logit(LOG_WARNING, "Interface %s has invalid IP# %s", ctx->check_interface, address);

		return RC_OS_INVALID_IP_ADDRESS;
	}

	for (i = 0; i < ctx->info_count; i++) {
		ddns_info_t *info = &ctx->info[i];

		for (j = 0; j < info->alias_count; j++) {
			ddns_alias_t *alias = &info->alias[j];

			alias->ip_has_changed = strcmp(alias->address, address) != 0;
			if (alias->ip_has_changed) {
				anychange++;
				strcpy(alias->address, address);
			}
		}
	}

	if (!anychange) {
		if (ctx->dbg.level > 0)
			logit(LOG_INFO, "No IP# change detected, still at %s", address);
	}

	return 0;
}

static int get_req_for_ip_server(ddns_t *ctx, ddns_info_t *info)
{
	return snprintf(ctx->request_buf, ctx->request_buflen,
			DYNDNS_CHECKIP_HTTP_REQUEST, info->checkip_url,
			info->checkip_name.name);
}

/*
	Send req to IP server and get the response
*/
static int server_transaction(ddns_t *ctx, int servernum)
{
	int rc = 0;
	http_trans_t *trans;
	http_t *http = &ctx->http_to_ip_server[servernum];
	ddns_info_t *info = &ctx->info[servernum];

	http->verbose = ctx->dbg.level;
	http->ssl_enabled = (info->checkip_name.port == HTTPS_DEFAULT_PORT) ? 1 : 0;
	DO(http_init(http, "Checking for IP# change"));

	/* Prepare request for IP server */
	memset(ctx->work_buf, 0, ctx->work_buflen);
	memset(ctx->request_buf, 0, ctx->request_buflen);
	memset(&ctx->http_transaction, 0, sizeof(ctx->http_transaction));

	trans              = &ctx->http_transaction;
	trans->req_len     = get_req_for_ip_server(ctx, &ctx->info[servernum]);
	trans->p_req       = ctx->request_buf;
	trans->p_rsp       = ctx->work_buf;
	trans->max_rsp_len = ctx->work_buflen - 1;	/* Save place for terminating \0 in string. */

	if (ctx->dbg.level > 1)
		logit(LOG_DEBUG, "Querying DDNS checkip server for my public IP#: %s", ctx->request_buf);

	rc = http_transaction(http, &ctx->http_transaction);
	if (trans->status != 200)
		rc = RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER;

	http_exit(http);
	if (ctx->dbg.level > 1)
		logit(LOG_DEBUG, "Checked my IP, return code: %d", rc);

	return rc;
}

/*
  Read in 4 integers the ip addr numbers.
  construct then the IP address from those numbers.
  Note:
  it updates the flag: alias->ip_has_changed if the old address was different
*/
static int parse_my_ip_address(ddns_t *ctx, int UNUSED(servernum))
{
	int found = 0;
	char *accept = "0123456789.";
	char *needle, *haystack, *end;
	struct in_addr addr;

	if (!ctx || ctx->http_transaction.rsp_len <= 0 || !ctx->http_transaction.p_rsp)
		return RC_INVALID_POINTER;

	haystack = ctx->http_transaction.p_rsp_body;
	needle = haystack;
	end = haystack + strlen(haystack) - 1;

	while (needle && haystack < end) {
		/* Try to find first decimal number (begin of IP) */
		needle = strpbrk(haystack, accept);
		if (needle) {
			size_t len = strspn(needle, accept);

			if (len)
				needle[len] = 0;

			if (!inet_aton(needle, &addr)) {
				haystack = needle + len + 1;
				continue;
			}

			/* FIRST occurence of a valid IP found */
			found = 1;
			break;
		}
	}

	if (found) {
		int i, anychange = 0;
		char *address = inet_ntoa(addr);

		for (i = 0; i < ctx->info_count; i++) {
			int j;
			ddns_info_t *info = &ctx->info[i];

			for (j = 0; j < info->alias_count; j++) {
				ddns_alias_t *alias = &info->alias[j];

				alias->ip_has_changed = strcmp(alias->address, address) != 0;
				if (alias->ip_has_changed) {
					anychange++;
					strcpy(alias->address, address);
				}
			}
		}

		if (!anychange) {
			if (ctx->dbg.level > 0)
				logit(LOG_INFO, "No IP# change detected, still at %s", address);
		} else {
			if (ctx->dbg.level > 0)
				logit(LOG_INFO, "Current public IP# %s", address);
		}

		return 0;
	}

	return RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER;
}

static int time_to_check(ddns_t *ctx, ddns_alias_t *alias)
{
	time_t past_time = time(NULL) - alias->last_update;

	return ctx->force_addr_update ||
		(past_time > ctx->forced_update_period_sec);
}

/*
  Updates for every maintained name the property: 'update_required'.
  The property will be checked in another function and updates performed.

  Action:
  Check if my IP address has changed. -> ALL names have to be updated.
  Nothing else.
  Note: In the update function the property will set to false if update was successful.
*/
static int check_alias_update_table(ddns_t *ctx)
{
	int i, j;

	/* Uses fix test if ip of server 0 has changed.
	 * That should be OK even if changes check_address() to
	 * iterate over servernum, but not if it's fix set to =! 0 */
	for (i = 0; i < ctx->info_count; i++) {
		ddns_info_t *info = &ctx->info[i];

		for (j = 0; j < info->alias_count; j++) {
			int override;
			ddns_alias_t *alias = &info->alias[j];

/* XXX: TODO time_to_check() will return false positive if the cache
 *     file is missing => causing unnnecessary update.  We should save
 *     the cache file with the current IP instead and fall back to
 *     standard update interval!
 */
			override = time_to_check(ctx, alias);
			if (!alias->ip_has_changed && !override) {
				alias->update_required = 0;
				continue;
			}

			alias->update_required = 1;
			logit(LOG_WARNING, "Update %s for alias %s, new IP# %s",
			      override ? "forced" : "needed", alias->name, alias->address);
		}
	}

	return 0;
}

static int send_update(ddns_t *ctx, ddns_info_t *info, ddns_alias_t *alias, int *changed)
{
	int            rc;
	http_trans_t   trans;
	http_t        *client = &ctx->http_to_dyndns[info->id];

	client->verbose = ctx->dbg.level;
	client->ssl_enabled = info->ssl_enabled;
	if (info->server_name.port == HTTPS_DEFAULT_PORT)
		client->ssl_enabled = 1;
	DO(http_init(client, "Sending IP# update to DDNS server"));

	memset(ctx->work_buf, 0, ctx->work_buflen);
	memset(ctx->request_buf, 0, ctx->request_buflen);
	memset(&trans, 0, sizeof(trans));

	trans.req_len     = info->system->request(ctx, info, alias);
	trans.p_req       = (char *)ctx->request_buf;
	trans.p_rsp       = (char *)ctx->work_buf;
	trans.max_rsp_len = ctx->work_buflen - 1;	/* Save place for a \0 at the end */

	if (ctx->dbg.level > 1) {
		ctx->request_buf[trans.req_len] = 0;
		logit(LOG_DEBUG, "Sending alias table update to DDNS server: %s", ctx->request_buf);
	}

	rc = http_transaction(client, &trans);

	if (ctx->dbg.level > 1)
		logit(LOG_DEBUG, "DDNS server response: %s", trans.p_rsp);

	if (rc) {
		http_exit(client);
		return rc;
	}

	rc = info->system->response(&trans, info, alias);
	if (rc) {
		logit(LOG_WARNING, "%s error in DDNS server response:",
		      rc == RC_DYNDNS_RSP_RETRY_LATER ? "Temporary" : "Fatal");
		logit(LOG_WARNING, "[%d %s] %s", trans.status, trans.status_desc,
		      trans.p_rsp_body != trans.p_rsp ? trans.p_rsp_body : "");
	} else {
		logit(LOG_INFO, "Successful alias table update for %s => new IP# %s",
		      alias->name, alias->address);

		ctx->force_addr_update = 0;
		if (changed)
			(*changed)++;
	}

	http_exit(client);

	return rc;
}

static int update_alias_table(ddns_t *ctx)
{
	int i, j;
	int rc = 0, remember = RC_OK;
	int anychange = 0;

	/* Issue #15: On external trig. force update to random addr. */
	if (ctx->force_addr_update && ctx->forced_update_fake_addr) {
		/* If the DDNS server responds with an error, we ignore it here,
		 * since this is just to fool the DDNS server to register a a
		 * change, i.e., an active user. */
		for (i = 0; i < ctx->info_count; i++) {
			ddns_info_t *info = &ctx->info[i];

			for (j = 0; j < info->alias_count; j++) {
				char backup[SERVER_NAME_LEN];
				ddns_alias_t *alias = &info->alias[j];

				strcpy(backup, alias->address);

				/* TODO: Use random address in 203.0.113.0/24 instead */
				snprintf(alias->address, sizeof(alias->address), "203.0.113.42");
				TRY(send_update(ctx, info, alias, NULL));

				strcpy(alias->address, backup);
			}
		}

		os_sleep_ms(1000);
	}

	for (i = 0; i < ctx->info_count; i++) {
		ddns_info_t *info = &ctx->info[i];

		for (j = 0; j < info->alias_count; j++) {
			ddns_alias_t *alias = &info->alias[j];

			if (!alias->update_required)
				continue;

			TRY(send_update(ctx, info, alias, &anychange));

			/* Only reset if send_update() succeeds. */
			alias->update_required = 0;
			alias->last_update = time(NULL);

			/* Update cache file for this entry */
			write_cache_file(alias);

			/* Run external command hook on update. */
			if (ctx->external_command)
				os_shell_execute(ctx->external_command,
						 alias->address, alias->name,
						 ctx->bind_interface);
		}

		if (rc != RC_OK && rc != RC_DYNDNS_RSP_RETRY_LATER)
			remember = rc;

		if (rc == RC_DYNDNS_RSP_RETRY_LATER && remember == RC_OK)
			remember = rc;
	}

	return remember;
}

static int get_encoded_user_passwd(ddns_t *ctx)
{
	int i, rc = 0;
	char buf[1024] = {0};
	size_t len, pwlen;

	for (i = 0; i < ctx->info_count; i++) {
		int rc2;
		char *encode;
		size_t dlen = 0;
		ddns_info_t *info = &ctx->info[i];

		info->creds.encoded = 0;

		/* Concatenate username and password with a ':', without
		 * snprintf(), since that can cause information loss if
		 * the password has "\=" or similar in it, issue #57 */
		len = MIN(strlen(info->creds.username), sizeof(buf)-3);
		if (len > 0)
			strncpy(buf, info->creds.username, len);
		buf[len] = 0;
		strcat(buf, ":");
		pwlen = strlen(buf);
		len = MIN(strlen(info->creds.password), sizeof(buf)-pwlen-1);
		if (len > 0)
			strncpy(&buf[pwlen], info->creds.password, len);
		buf[pwlen+len] = 0;

		/* query required buffer size for base64 encoded data */
		base64_encode(NULL, &dlen, (unsigned char *)buf, strlen(buf));
		encode = malloc(dlen);
		if (!encode) {
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		/* encode */
		rc2 = base64_encode((unsigned char *)encode, &dlen, (unsigned char *)buf, strlen(buf));
		if (rc2 != 0) {
			free(encode);
			rc = RC_OUT_BUFFER_OVERFLOW;
			break;
		}

		info->creds.encoded_password = encode;
		info->creds.encoded = 1;
		info->creds.size = strlen(info->creds.encoded_password);
	}

	return rc;
}

/**
   Sets up the object.
   - sets the IPs of the DYN DNS server
   - if proxy server is set use it!
   - ...
*/
static int init_context(ddns_t *ctx)
{
	int i;

	if (!ctx)
		return RC_INVALID_POINTER;

	if (ctx->initialized == 1)
		return 0;

	for (i = 0; i < ctx->info_count; i++) {
		http_t      *checkip = &ctx->http_to_ip_server[i];
		http_t      *update  = &ctx->http_to_dyndns[i];
		ddns_info_t *info    = &ctx->info[i];

		if (strlen(info->proxy_server_name.name)) {
			http_set_port(checkip, info->proxy_server_name.port);
			http_set_port(update,  info->proxy_server_name.port);

			http_set_remote_name(checkip, info->proxy_server_name.name);
			http_set_remote_name(update,  info->proxy_server_name.name);
		} else {
			http_set_port(checkip, info->checkip_name.port);
			http_set_port(update,  info->server_name.port);

			http_set_remote_name(checkip, info->checkip_name.name);
			http_set_remote_name(update,  info->server_name.name);
		}

		http_set_bind_iface(update,  ctx->bind_interface);
		http_set_bind_iface(checkip, ctx->bind_interface);
	}

	/* Restore values, if reset by SIGHUP.  Initialize time from cache file at startup. */
	ctx->num_iterations = cached_num_iterations;

	ctx->initialized = 1;

	return 0;
}

/** the real action:
    - increment the forced update times counter
    - detect current IP
    - connect to an HTTP server
    - parse the response for IP addr

    - for all the names that have to be maintained
    - get the current DYN DNS address from DYN DNS server
    - compare and update if neccessary
*/
static int check_address(ddns_t *ctx)
{
	if (!ctx)
		return RC_INVALID_POINTER;

	if (ctx->check_interface) {
		DO(check_interface_address(ctx));
	} else {
		int servernum = 0;  /* server to use for checking IP, index 0 by default */

		/* Ask IP server something so he will respond and give me my IP */
		DO(server_transaction(ctx, servernum));
		if (ctx->dbg.level > 1) {
			logit(LOG_DEBUG, "IP server response:");
			logit(LOG_DEBUG, "%s", ctx->work_buf);
		}

		/* Extract our IP, check if different than previous one */
		DO(parse_my_ip_address(ctx, servernum));
	}

	/* Step through aliases list, resolve them and check if they point to my IP */
	DO(check_alias_update_table(ctx));

	/* Update IPs marked as not identical with my IP */
	DO(update_alias_table(ctx));

	return 0;
}

/* Error filter.  Some errors are to be expected in a network
 * application, some we can recover from, wait a shorter while and try
 * again, whereas others are terminal, e.g., some OS errors. */
static int check_error(ddns_t *ctx, int rc)
{
	switch (rc) {
	case RC_OK:
		ctx->sleep_sec = ctx->normal_update_period_sec;
		break;

	/* dyn_dns_update_ip() failed, inform the user the (network) error
	 * is not fatal and that we will retry again in a short while. */
	case RC_DYNDNS_RSP_NOTOK:
	case RC_DYNDNS_RSP_RETRY_LATER:
		ctx->sleep_sec = ctx->error_update_period_sec;
		break;

	case RC_IP_INVALID_REMOTE_ADDR: /* Probably temporary DNS error. */
	case RC_IP_CONNECT_FAILED:      /* Cannot connect to DDNS server atm. */
	case RC_IP_SEND_ERROR:
	case RC_IP_RECV_ERROR:
	case RC_HTTPS_NO_SSL_SUPPORT:
#ifdef ENABLE_SSL
	case RC_HTTPS_SNI_ERROR:
	case RC_HTTPS_FAILED_CONNECT:
	case RC_HTTPS_FAILED_GETTING_CERT:
	case RC_HTTPS_SEND_ERROR:
	case RC_HTTPS_RECV_ERROR:
#endif
	case RC_OS_INVALID_IP_ADDRESS:
	case RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER:
		ctx->sleep_sec = DYNDNS_FAILED_UPDATE_PERIOD;
		break;

	/* All other errors, socket creation failures, invalid pointers etc.  */
	default:
		logit(LOG_ERR, "Unrecoverable error %d, exiting ...", rc);
		return 1;
	}

	return 0;
}

/**
 * Main DDNS loop
 * Actions:
 *  - read the configuration options
 *  - perform various init actions as specified in the options
 *  - create and init dyn_dns object.
 *  - launch the IP update action loop
 */
int ddns_main_loop(ddns_t *ctx, int argc, char *argv[])
{
	int rc = 0;
	int iterations_err = 0;
	static int first_startup = 1;

	if (!ctx)
		return RC_INVALID_POINTER;

	/* read cmd line options and set object properties */
	rc = get_config_data(ctx, argc, argv);
	if (rc != 0 || ctx->abort)
		return rc;

	if (ctx->change_persona) {
		ddns_user_t user;

		memset(&user, 0, sizeof(user));
		user.gid = ctx->sys_usr_info.gid;
		user.uid = ctx->sys_usr_info.uid;

		DO(os_change_persona(&user));
	}

	/* if silent required, close console window */
	if (ctx->run_in_background == 1) {
		DO(close_console_window());

		if (get_dbg_dest() == DBG_SYS_LOG)
			fclose(stdout);
	}

	/* Check file system permissions and create pidfile */
	if (!ctx->update_once)
		DO(os_check_perms(ctx));

	/* if logfile provided, redirect output to log file */
	if (strlen(ctx->dbg.p_logfilename) != 0)
		DO(os_open_dbg_output(DBG_FILE_LOG, "", ctx->dbg.p_logfilename));

	if (ctx->debug_to_syslog == 1 || (ctx->run_in_background == 1)) {
		if (get_dbg_dest() == DBG_STD_LOG)	/* avoid file and syslog output */
			DO(os_open_dbg_output(DBG_SYS_LOG, "inadyn", NULL));
	}

	/* "Hello!" Let user know we've started up OK */
	logit(LOG_INFO, "%s", VERSION_STRING);

	/* On first startup only, optionally wait for network and any NTP daemon
	 * to set system time correctly.  Intended for devices without battery
	 * backed real time clocks as initialization of time since last update
	 * requires the correct time.  Sleep can be interrupted with the usual
	 * signals inadyn responds too. */
	if (first_startup && ctx->startup_delay_sec) {
		if (ctx->dbg.level > 1)
			logit(LOG_NOTICE, "Startup delay: %d sec ...", ctx->startup_delay_sec);
		first_startup = 0;

		/* Now sleep a while. Using the time set in sleep_sec data member */
		ctx->sleep_sec = ctx->startup_delay_sec;
		wait_for_cmd(ctx);

		if (ctx->cmd == CMD_STOP) {
			if (ctx->dbg.level > 1)
				logit(LOG_INFO, "STOP command received, exiting.");
			return 0;
		}
		if (ctx->cmd == CMD_RESTART) {
			if (ctx->dbg.level > 1)
				logit(LOG_INFO, "RESTART command received, restarting.");
			return RC_RESTART;
		}
		if (ctx->cmd == CMD_FORCED_UPDATE) {
			logit(LOG_INFO, "FORCED_UPDATE command received, updating now.");
			ctx->force_addr_update = 1;
			ctx->cmd = NO_CMD;
		} else if (ctx->cmd == CMD_CHECK_NOW) {
			logit(LOG_INFO, "CHECK_NOW command received, leaving startup delay.");
			ctx->cmd = NO_CMD;
		}
	}

	DO(init_context(ctx));
	DO(read_cache_file(ctx));
	DO(get_encoded_user_passwd(ctx));

	if (ctx->update_once == 1)
		ctx->force_addr_update = 1;

	/* DDNS client main loop */
	while (1) {
		rc = check_address(ctx);
		if (rc == RC_OK) {
			iterations_err = 0;
			if (ctx->total_iterations != 0 &&
			    ++ctx->num_iterations >= ctx->total_iterations)
				break;
		} else {
			iterations_err++;
		}

		if (ctx->cmd == CMD_RESTART) {
			if (ctx->dbg.level > 1)
				logit(LOG_INFO, "RESTART command received. Restarting.");
			ctx->cmd = NO_CMD;
			rc = RC_RESTART;
			break;
		}

		/* On error, check why, possibly need to retry sooner ... */
		if (check_error(ctx, rc))
			break;

		/* prevent infinity loop on fixed number iterations after many failed update */
		if (ctx->total_iterations > 0 && iterations_err > ctx->total_iterations)
			break;

		/* check special case for ONCE mode */
		if (ctx->update_once == 1) {
			/* terminate now after server kick ass */
			if (rc == RC_DYNDNS_RSP_NOTOK ||
			    rc == RC_DYNDNS_INVALID_RSP_FROM_IP_SERVER)
				break;
			
			/* else minimal sleep for try again */
			ctx->sleep_sec = DYNDNS_MIN_SLEEP;
		} else {
			/* dynamic increase interval after 3x errors (upper limit 60 min) */
			if (iterations_err > 3) {
				int sleep_sec = ctx->sleep_sec * (iterations_err - 2);
				if (sleep_sec > 3600)
					sleep_sec = 3600;
				ctx->sleep_sec = sleep_sec;
			}
		}

		if (rc != RC_OK) {
			int minutes = ctx->sleep_sec / 60;
			if (!minutes)
				logit(LOG_WARNING, "Will retry again after %d %s...", ctx->sleep_sec, "sec");
			else
				logit(LOG_WARNING, "Will retry again after %d %s...", minutes, "min");
		}

		/* Now sleep a while. Using the time set in sleep_sec data member */
		wait_for_cmd(ctx);

		if (ctx->cmd == CMD_STOP) {
			if (ctx->dbg.level > 1)
				logit(LOG_INFO, "STOP command received, exiting.");
			rc = 0;
			break;
		}
		if (ctx->cmd == CMD_RESTART) {
			if (ctx->dbg.level > 1)
				logit(LOG_INFO, "RESTART command received, restarting.");
			rc = RC_RESTART;
			break;
		}
		if (ctx->cmd == CMD_FORCED_UPDATE) {
			logit(LOG_INFO, "FORCED_UPDATE command received, updating now.");
			ctx->force_addr_update = 1;
			ctx->cmd = NO_CMD;
			continue;
		}

		if (ctx->cmd == CMD_CHECK_NOW) {
			logit(LOG_INFO, "CHECK_NOW command received, checking ...");
			ctx->cmd = NO_CMD;
			continue;
		}
	}

	/* Save old value, if restarted by SIGHUP */
	cached_num_iterations = ctx->num_iterations;

	return rc;
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
