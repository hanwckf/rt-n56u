/*
 * Copyright (C) 2010 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/* /etc/services
#   service-name   port/protocol   [aliases ...]
discard               9/udp        sink null

service-name: case sensitive friendly name of the service
port: decimal port number
protocol: protocols(5) compatible entry
aliases: case sensitive optional space or tab separated list of other names
*/

#include <features.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "internal/parse_config.h"

#include <bits/uClibc_mutex.h>
__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

#define MINTOKENS	3
#define	MAXALIASES	8	/* we seldomly need more than 1 alias */
#define MAXTOKENS	(MINTOKENS + MAXALIASES + 1)
#define BUFSZ		(255)	/* one line */
#define SBUFSIZE	(BUFSZ + 1 + (sizeof(char *) * MAXTOKENS))

static parser_t *servp = NULL;
static struct servent serve;
static char *servbuf = NULL;
static size_t servbuf_sz = SBUFSIZE;
static smallint serv_stayopen;

void setservent(int stayopen)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (servp)
		config_close(servp);
	servp = config_open(_PATH_SERVICES);
	if (stayopen)
		serv_stayopen = 1;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(setservent)

void endservent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (servp) {
		config_close(servp);
		servp = NULL;
	}
	serv_stayopen = 0;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(endservent)

int getservent_r(struct servent *result_buf,
				 char *buf, size_t buflen, struct servent **result)
{
	char **tok = NULL;
	const size_t aliaslen = sizeof(char *) * MAXTOKENS;
	int ret = ERANGE;

	*result = NULL;
	if (buflen < aliaslen
		|| (buflen - aliaslen) < BUFSZ + 1)
		goto DONE_NOUNLOCK;

	__UCLIBC_MUTEX_LOCK(mylock);
	ret = ENOENT;
	if (servp == NULL)
		setservent(serv_stayopen);
	if (servp == NULL)
		goto DONE;

	servp->data = buf;
	servp->data_len = aliaslen;
	servp->line_len = buflen - aliaslen;
	/* <name>[[:space:]]<port>/<proto>[[:space:]][<aliases>] */
	if (!config_read(servp, &tok, MAXTOKENS - 1, MINTOKENS, "# \t/", PARSE_NORMAL)) {
		goto DONE;
	}
	result_buf->s_name = *(tok++);
	result_buf->s_port = htons((u_short) atoi(*(tok++)));
	result_buf->s_proto = *(tok++);
	result_buf->s_aliases = tok;
	*result = result_buf;
	ret = 0;
 DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
 DONE_NOUNLOCK:
	errno = ret;
	return errno;
}
libc_hidden_def(getservent_r)

static void __initbuf(void)
{
	if (!servbuf)
		servbuf = malloc(SBUFSIZE);
	if (!servbuf)
		abort();
}

struct servent *getservent(void)
{
	struct servent *result;

	__initbuf();
	getservent_r(&serve, servbuf, servbuf_sz, &result);
	return result;
}

int getservbyname_r(const char *name, const char *proto,
					struct servent *result_buf, char *buf, size_t buflen,
					struct servent **result)
{
	register char **cp;
	int ret;

	__UCLIBC_MUTEX_LOCK(mylock);
	setservent(serv_stayopen);
	while (!(ret = getservent_r(result_buf, buf, buflen, result))) {
		if (strcmp(name, result_buf->s_name) == 0)
			goto gotname;
		for (cp = result_buf->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
 gotname:
		if (proto == 0 || strcmp(result_buf->s_proto, proto) == 0)
			break;
	}
	if (!serv_stayopen)
		endservent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getservbyname_r)

struct servent *getservbyname(const char *name, const char *proto)
{
	struct servent *result;

	__initbuf();
	getservbyname_r(name, proto, &serve, servbuf, servbuf_sz, &result);
	return result;
}


int getservbyport_r(int port, const char *proto,
					struct servent *result_buf, char *buf,
					size_t buflen, struct servent **result)
{
	int ret;

	__UCLIBC_MUTEX_LOCK(mylock);
	setservent(serv_stayopen);
	while (!(ret = getservent_r(result_buf, buf, buflen, result))) {
		if (result_buf->s_port != port)
			continue;
		if (proto == 0 || strcmp(result_buf->s_proto, proto) == 0)
			break;
	}
	if (!serv_stayopen)
		endservent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getservbyport_r)

struct servent *getservbyport(int port, const char *proto)
{
	struct servent *result;

	__initbuf();
	getservbyport_r(port, proto, &serve, servbuf, servbuf_sz, &result);
	return result;
}
libc_hidden_def(getservbyport)
