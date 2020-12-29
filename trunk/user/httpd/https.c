/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/dh.h>

#include "httpd.h"

#define SYSLOG_ID_SSL	"SSL/TLS"

typedef struct {
	SSL* ssl;
	int fd;
} http_ssl_cookie_t;


extern int debug_mode;
static SSL_CTX *ssl_ctx = NULL;

/* 1024-bit MODP Group with 160-bit prime order subgroup (RFC5114)
* -----BEGIN DH PARAMETERS-----
* MIIBDAKBgQCxC4+WoIDgHd6S3l6uXVTsUsmfvPsGo8aaap3KUtI7YWBz4oZ1oj0Y
* mDjvHi7mUsAT7LSuqQYRIySXXDzUm4O/rMvdfZDEvXCYSI6cIZpzck7/1vrlZEc4
* +qMaT/VbzMChUa9fDci0vUW/N982XBpl5oz9p21NpwjfH7K8LkpDcQKBgQCk0cvV
* w/00EmdlpELvuZkF+BBN0lisUH/WQGz/FCZtMSZv6h5cQVZLd35pD1UE8hMWAhe0
* sBuIal6RVH+eJ0n01/vX07mpLuGQnQ0iY/gKdqaiTAh6CR9THb8KAWm2oorWYqTR
* jnOvoy13nVkY0IvIhY9Nzvl8KiSFXm7rIrOy5QICAKA=
* -----END DH PARAMETERS-----
*/

static const unsigned char dh1024_p[]={
	0xB1,0x0B,0x8F,0x96,0xA0,0x80,0xE0,0x1D,0xDE,0x92,0xDE,0x5E,
	0xAE,0x5D,0x54,0xEC,0x52,0xC9,0x9F,0xBC,0xFB,0x06,0xA3,0xC6,
	0x9A,0x6A,0x9D,0xCA,0x52,0xD2,0x3B,0x61,0x60,0x73,0xE2,0x86,
	0x75,0xA2,0x3D,0x18,0x98,0x38,0xEF,0x1E,0x2E,0xE6,0x52,0xC0,
	0x13,0xEC,0xB4,0xAE,0xA9,0x06,0x11,0x23,0x24,0x97,0x5C,0x3C,
	0xD4,0x9B,0x83,0xBF,0xAC,0xCB,0xDD,0x7D,0x90,0xC4,0xBD,0x70,
	0x98,0x48,0x8E,0x9C,0x21,0x9A,0x73,0x72,0x4E,0xFF,0xD6,0xFA,
	0xE5,0x64,0x47,0x38,0xFA,0xA3,0x1A,0x4F,0xF5,0x5B,0xCC,0xC0,
	0xA1,0x51,0xAF,0x5F,0x0D,0xC8,0xB4,0xBD,0x45,0xBF,0x37,0xDF,
	0x36,0x5C,0x1A,0x65,0xE6,0x8C,0xFD,0xA7,0x6D,0x4D,0xA7,0x08,
	0xDF,0x1F,0xB2,0xBC,0x2E,0x4A,0x43,0x71,
};

static const unsigned char dh1024_g[]={
	0xA4,0xD1,0xCB,0xD5,0xC3,0xFD,0x34,0x12,0x67,0x65,0xA4,0x42,
	0xEF,0xB9,0x99,0x05,0xF8,0x10,0x4D,0xD2,0x58,0xAC,0x50,0x7F,
	0xD6,0x40,0x6C,0xFF,0x14,0x26,0x6D,0x31,0x26,0x6F,0xEA,0x1E,
	0x5C,0x41,0x56,0x4B,0x77,0x7E,0x69,0x0F,0x55,0x04,0xF2,0x13,
	0x16,0x02,0x17,0xB4,0xB0,0x1B,0x88,0x6A,0x5E,0x91,0x54,0x7F,
	0x9E,0x27,0x49,0xF4,0xD7,0xFB,0xD7,0xD3,0xB9,0xA9,0x2E,0xE1,
	0x90,0x9D,0x0D,0x22,0x63,0xF8,0x0A,0x76,0xA6,0xA2,0x4C,0x08,
	0x7A,0x09,0x1F,0x53,0x1D,0xBF,0x0A,0x01,0x69,0xB6,0xA2,0x8A,
	0xD6,0x62,0xA4,0xD1,0x8E,0x73,0xAF,0xA3,0x2D,0x77,0x9D,0x59,
	0x18,0xD0,0x8B,0xC8,0x85,0x8F,0x4D,0xCE,0xF9,0x7C,0x2A,0x24,
	0x85,0x5E,0x6E,0xEB,0x22,0xB3,0xB2,0xE5,
};

static ssize_t
http_ssl_read(void *cookie, char *buf, size_t len)
{
	int nr, err;
	http_ssl_cookie_t *hsc = (http_ssl_cookie_t *)cookie;
	size_t total = 0;

	do {
		nr = SSL_read(hsc->ssl, buf + total, len - total);
		if (nr > 0) {
			total += (size_t)nr;
		} else {
			err = SSL_get_error(hsc->ssl, nr);
			switch (err) {
			case SSL_ERROR_ZERO_RETURN:
				goto read_out;
				break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				break;
			default:
				if (debug_mode)
					ERR_print_errors_fp(stderr);
				if (total == 0)
					total = (ssize_t)-1;
				goto read_out;
			}
		}
	} while ((total < len) && SSL_pending(hsc->ssl));

read_out:

	return (ssize_t)total;
}

static ssize_t
http_ssl_write(void *cookie, const char *buf, size_t len)
{
	int nw, err;
	http_ssl_cookie_t *hsc = (http_ssl_cookie_t *)cookie;
	size_t total = 0;

	while (total < len) {
		nw = SSL_write(hsc->ssl, buf + total, len - total);
		if (nw > 0) {
			total += (size_t)nw;
		} else {
			err = SSL_get_error(hsc->ssl, nw);
			switch (err) {
			case SSL_ERROR_ZERO_RETURN:
				goto write_out;
				break;
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_WANT_READ:
				break;
			default:
				if (debug_mode)
					ERR_print_errors_fp(stderr);
				if (total == 0)
					total = (ssize_t)-1;
				goto write_out;
			}
		}
	}

write_out:

	return (ssize_t)total;
}

static int
http_ssl_close(void *cookie)
{
	http_ssl_cookie_t *hsc = (http_ssl_cookie_t *)cookie;
	if (!hsc)
		return 0;

	if (hsc->ssl) {
		SSL_shutdown(hsc->ssl);
		shutdown(hsc->fd, SHUT_WR);
		SSL_free(hsc->ssl);
	}

	close(hsc->fd);

	free(hsc);

	return 0;
}

static const cookie_io_functions_t http_ssl_io = {
	.read  = http_ssl_read,
	.write = http_ssl_write,
	.seek  = NULL,
	.close = http_ssl_close
};

static void
http_ssl_info_cb(const SSL *ssl, int where, int ret)
{
	/* disable SSL renegotiation */
	if (where & SSL_CB_HANDSHAKE_DONE) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
		ssl->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
#else
		SSL_set_options(ssl, SSL_OP_NO_RENEGOTIATION);
#endif
	}
}

void ssl_server_uninit(void)
{
	if (ssl_ctx) {
		SSL_CTX_free(ssl_ctx);
		ssl_ctx = NULL;
	}

	ERR_free_strings();
}

int ssl_server_init(char* ca_file, char *crt_file, char *key_file, char *dhp_file, char *ssl_cipher_list)
{
	static const char *ssl_ctx_id = "httpd";
	long ssl_options;
	BIGNUM *p, *g;

	if (!crt_file || !f_exists(crt_file)) {
		httpd_log("%s: Server certificate (%s) is not found!", SYSLOG_ID_SSL, crt_file);
		httpd_log("Please manual build the certificate via \"%s\" script.", "https-cert.sh");
		return -1;
	}

	if (!key_file || !f_exists(key_file)) {
		httpd_log("%s: Server private key (%s) is not found!", SYSLOG_ID_SSL, key_file);
		httpd_log("Please manual build the certificate via \"%s\" script.", "https-cert.sh");
		return -1;
	}

	SSL_load_error_strings();
	SSL_library_init();

	ssl_ctx = SSL_CTX_new(SSLv23_server_method());
	if (!ssl_ctx) {
		httpd_log("%s: Unable to create SSL context!", SYSLOG_ID_SSL);
		return -1;
	}

	ssl_options = SSL_OP_ALL | SSL_OP_NO_COMPRESSION | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 |
			SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION;

	SSL_CTX_set_options(ssl_ctx, ssl_options);
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);

	if (ssl_cipher_list && strlen(ssl_cipher_list) > 2) {
		if (SSL_CTX_set_cipher_list(ssl_ctx, ssl_cipher_list) != 1) {
			httpd_log("%s: Cannot set SSL cipher list (%s)!", SYSLOG_ID_SSL, ssl_cipher_list);
		} else {
			SSL_CTX_set_options(ssl_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
		}
	}

	if (ca_file && f_exists(ca_file)) {
		if (SSL_CTX_load_verify_locations(ssl_ctx, ca_file, NULL) != 1) {
			httpd_log("%s: Cannot load CA certificate (%s)!", SYSLOG_ID_SSL, ca_file);
		}
	}

	if (SSL_CTX_use_certificate_file(ssl_ctx, crt_file, SSL_FILETYPE_PEM) != 1) {
		httpd_log("%s: Cannot load server certificate (%s)!", SYSLOG_ID_SSL, crt_file);
		ssl_server_uninit();
		return 1;
	}

	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key_file, SSL_FILETYPE_PEM) != 1) {
		httpd_log("%s: Cannot load server private key (%s)!", SYSLOG_ID_SSL, key_file);
		ssl_server_uninit();
		return 1;
	}

	if (SSL_CTX_check_private_key(ssl_ctx) != 1) {
		httpd_log("%s: Private key does not match the certificate!", SYSLOG_ID_SSL);
		ssl_server_uninit();
		return 1;
	}

	if (dhp_file && f_exists(dhp_file)) {
		/* DH parameters from file */
		BIO *bio = BIO_new_file(dhp_file, "r");
		if (bio) {
			DH *dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
			BIO_free(bio);
			if (dh) {
				SSL_CTX_set_tmp_dh(ssl_ctx, dh);
				SSL_CTX_set_options(ssl_ctx, SSL_OP_SINGLE_DH_USE);
				DH_free(dh);
			} else {
				httpd_log("%s: Cannot load DH parameters (%s)!", SYSLOG_ID_SSL, dhp_file);
			}
		}
	} else {
		/* Default DH parameters from RFC5114 */
		DH *dh = DH_new();
		if (dh) {
			p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
			g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
			dh->p = p;
			dh->g = g;
			dh->length = 160;
			if (dh->p && dh->g) {
#else
			if(DH_set0_pqg(dh, p, NULL, g) && DH_set_length(dh, 160)) {
#endif
				SSL_CTX_set_tmp_dh(ssl_ctx, dh);
				SSL_CTX_set_options(ssl_ctx, SSL_OP_SINGLE_DH_USE);
			}
			DH_free(dh);
		}
	}

	SSL_CTX_set_default_read_ahead(ssl_ctx, 1);

	SSL_CTX_set_session_cache_mode(ssl_ctx, SSL_SESS_CACHE_SERVER);
	SSL_CTX_set_session_id_context(ssl_ctx, (unsigned char *)ssl_ctx_id, strlen(ssl_ctx_id));
	SSL_CTX_sess_set_cache_size(ssl_ctx, 10);

	SSL_CTX_set_info_callback(ssl_ctx, http_ssl_info_cb);

	return 0;
}

FILE *ssl_server_fopen(int fd)
{
	FILE *fp;
	http_ssl_cookie_t *hsc;

	hsc = (http_ssl_cookie_t *)malloc(sizeof(http_ssl_cookie_t));
	if (!hsc)
		return NULL;

	hsc->fd = fd;
	hsc->ssl = SSL_new(ssl_ctx);
	if (!hsc->ssl)
		goto ERROR;

	SSL_set_mode(hsc->ssl, SSL_MODE_AUTO_RETRY);

	if (SSL_set_fd(hsc->ssl, fd) != 1)
		goto ERROR;

	for (;;) {
		int ret = SSL_accept(hsc->ssl);
		if (ret <= 0) {
			int err = SSL_get_error(hsc->ssl, ret);
			if (ret < 0 && (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE))
				continue;
			goto ERROR;
		}
		break;
	}

	fp = fopencookie((void*)hsc, "r+", http_ssl_io);
	if (!fp)
		goto ERROR;

	return fp;

ERROR:

	if (hsc->ssl)
		SSL_free(hsc->ssl);

	free(hsc);

	return NULL;
}

const char*
ssl_server_get_ssl_ver(void)
{
	return SSLeay_version(SSLEAY_VERSION);
}
