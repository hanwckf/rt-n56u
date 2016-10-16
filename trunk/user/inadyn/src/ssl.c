/* Interface for optional HTTPS functions
 *
 * Copyright (C) 2014-2015  Joachim Nilsson <troglobit@gmail.com>
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

#include "debug.h"
#include "ssl.h"

#ifdef ENABLE_SSL
/* SSL SNI support: tell the servername we want to speak to */
static int set_server_name(SSL *ssl, const char *sn)
{
	int rc = 0;

#if defined(CONFIG_OPENSSL)
	/* api returns 1 for success */
	rc = !SSL_set_tlsext_host_name(ssl, sn);
#endif

	return rc;
}
#endif

int ssl_init(http_t *client, char *msg)
{
#ifndef ENABLE_SSL
	(void)client;
	(void)msg;
	return 0;
#else
	int err, err_ssl, rc = 0;
	char buf[256];
	const char *sn;
	X509 *cert;

	if (client->verbose > 1)
		logit(LOG_INFO, "%s, initiating HTTPS ...", msg);

	do {
		client->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
		if (!client->ssl_ctx)
			return RC_HTTPS_OUT_OF_MEMORY;

#if defined(CONFIG_OPENSSL)
		/* POODLE, only allow TLSv1.x or later */
		SSL_CTX_set_options(client->ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);
#endif

		client->ssl = SSL_new(client->ssl_ctx);
		if (!client->ssl) {
			rc = RC_HTTPS_OUT_OF_MEMORY;
			break;
		}

		http_get_remote_name(client, &sn);
		if (set_server_name(client->ssl, sn)) {
			rc = RC_HTTPS_SNI_ERROR;
			break;
		}

		SSL_set_fd(client->ssl, client->tcp.ip.socket);
		err = SSL_connect(client->ssl);
		if (err <= 0) {
			err_ssl = SSL_get_error(client->ssl, err);
			logit(LOG_ERR, "SSL_connect %s! (err: %d)", "FAILED", err_ssl);
			rc = RC_HTTPS_FAILED_CONNECT;
			break;
		}

		if (client->verbose > 0)
			logit(LOG_INFO, "SSL connection using %s", SSL_get_cipher(client->ssl));

		/* Get server's certificate (note: beware of dynamic allocation) - opt */
		cert = SSL_get_peer_certificate(client->ssl);
		if (!cert) {
			logit(LOG_ERR, "SSL_get_peer_certificate %s!", "FAILED");
			rc = RC_HTTPS_FAILED_GETTING_CERT;
			break;
		}

		/* Logging some cert details. Please note: X509_NAME_oneline doesn't
		   work when giving NULL instead of a buffer. */
		buf[0] = 0;
		X509_NAME_oneline(X509_get_subject_name(cert), buf, sizeof(buf));
		if (client->verbose > 1)
			logit(LOG_INFO, "SSL server cert subject: %s", buf);

		buf[0] = 0;
		X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
		if (client->verbose > 1)
			logit(LOG_INFO, "SSL server cert issuer: %s", buf);

		/* We could do all sorts of certificate verification stuff here before
		   deallocating the certificate. */
		X509_free(cert);
	} while (0);

	if (rc) {
		ssl_exit(client);
		return rc;
	}

	return 0;
#endif
}

int ssl_exit(http_t *client)
{
#ifndef ENABLE_SSL
	(void)client;
	return 0;
#else
	if (client->ssl) {
		/* SSL/TLS close_notify */
		SSL_shutdown(client->ssl);
		
		/* Clean up. */
		SSL_free(client->ssl);
		client->ssl = NULL;
	}

	if (client->ssl_ctx) {
		/* Clean up. */
		SSL_CTX_free(client->ssl_ctx);
		client->ssl_ctx = NULL;
	}

	return 0;
#endif
}

#ifdef ENABLE_SSL
static ssize_t
ssl_read_socket(SSL* ssl, char *buf, size_t len)
{
	int nr, err;
	size_t total = 0;

	do {
		nr = SSL_read(ssl, buf + total, len - total);
		if (nr > 0) {
			total += (size_t)nr;
		} else {
			err = SSL_get_error(ssl, nr);
			switch (err) {
			case SSL_ERROR_ZERO_RETURN:
				goto read_out;
				break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				break;
			case SSL_ERROR_SYSCALL:
				if (ERR_get_error() == 0)
					goto read_out;
			default:
				if (total == 0)
					total = (ssize_t)-1;
				logit(LOG_ERR, "SSL_read %s! (err: %d)", "FAILED", err);
				goto read_out;
			}
		}
	} while ((total < len) && SSL_pending(ssl));

read_out:

	return (ssize_t)total;
}

static ssize_t
ssl_write_socket(SSL* ssl, const char *buf, size_t len)
{
	int nw, err;
	size_t total = 0;

	while (total < len) {
		nw = SSL_write(ssl, buf + total, len - total);
		if (nw > 0) {
			total += (size_t)nw;
		} else {
			err = SSL_get_error(ssl, nw);
			switch (err) {
			case SSL_ERROR_ZERO_RETURN:
				goto write_out;
				break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				break;
			case SSL_ERROR_SYSCALL:
				if (ERR_get_error() == 0)
					goto write_out;
			default:
				if (total == 0)
					total = (ssize_t)-1;
				logit(LOG_ERR, "SSL_write %s! (err: %d)", "FAILED", err);
				goto write_out;
			}
		}
	}

write_out:

	return (ssize_t)total;
}
#endif

int ssl_send(http_t *client, const char *buf, int len)
{
#ifndef ENABLE_SSL
	(void)client;
	(void)buf;
	(void)len;
	return RC_HTTPS_NO_SSL_SUPPORT;
#else
	ssize_t nw;

	nw = ssl_write_socket(client->ssl, buf, len);
	if (nw <= 0)
		return RC_HTTPS_SEND_ERROR;

	if (client->verbose > 1)
		logit(LOG_DEBUG, "Successfully sent DDNS update using HTTPS!");

	return 0;
#endif
}

int ssl_recv(http_t *client, char *buf, int buf_len, int *recv_len)
{
#ifndef ENABLE_SSL
	(void)client;
	(void)buf;
	(void)buf_len;
	(void)recv_len;
	return RC_HTTPS_NO_SSL_SUPPORT;
#else
	ssize_t nr;

	/* Read HTTP header & body */
	nr = ssl_read_socket(client->ssl, buf, buf_len);
	if (nr <= 0) {
		*recv_len = 0;
		return RC_HTTPS_RECV_ERROR;
	}

	*recv_len = nr;

	/* Read HTTP body */
	buf_len -= nr;
	if (buf_len > 0) {
		buf += nr;
		nr = ssl_read_socket(client->ssl, buf, buf_len);
		if (nr > 0)
			*recv_len += nr;
	}

	if (client->verbose > 1)
		logit(LOG_DEBUG, "Successfully received DDNS update response (%d bytes) using HTTPS!", *recv_len);

	return 0;
#endif
}

/**
 * Local Variables:
 *  version-control: t
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
