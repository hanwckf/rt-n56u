/*
 * Copyright (C) 2010 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/* /etc/protocols
#   protocol-name   number   [aliases ...]
ip                  0        IP  # internet protocol, pseudo protocol number

protocol-name: case sensitive friendly name of the IP protocol
number: decimal protocol number
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

#define MINTOKENS	2
#define	MAXALIASES	8 /* will probably never be more than one */
#define MAXTOKENS	(MINTOKENS + MAXALIASES + 1)
#define BUFSZ		(255) /* one line */
#define SBUFSIZE	(BUFSZ + 1 + (sizeof(char *) * MAXTOKENS))

static parser_t *protop = NULL;
static struct protoent protoe;
static char *protobuf = NULL;
static smallint proto_stayopen;

void setprotoent(int stayopen)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (protop)
		config_close(protop);
	protop = config_open(_PATH_PROTOCOLS);
	if (stayopen)
		proto_stayopen = 1;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(setprotoent)

void endprotoent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (protop) {
		config_close(protop);
		protop = NULL;
	}
	proto_stayopen = 0;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(endprotoent)

int getprotoent_r(struct protoent *result_buf,
				 char *buf, size_t buflen, struct protoent **result)
{
	char **tok = NULL;
	const size_t aliaslen = sizeof(char *) * MAXTOKENS;
	int ret = ERANGE;

	*result = NULL;
	if (buflen < aliaslen
		|| (buflen - aliaslen) < BUFSZ + 1)
		goto DONE_NOUNLOCK;

	__UCLIBC_MUTEX_LOCK(mylock);
	//tok = (char **) buf;
	ret = ENOENT;
	if (protop == NULL)
		setprotoent(proto_stayopen);
	if (protop == NULL)
		goto DONE;
	protop->data = buf;
	protop->data_len = aliaslen;
	protop->line_len = buflen - aliaslen;
	/* <name>[[:space:]]<protonumber>[[:space:]][<aliases>] */
	if (!config_read(protop, &tok, MAXTOKENS - 1, MINTOKENS, "# \t/", PARSE_NORMAL)) {
		goto DONE;
	}
	result_buf->p_name = *(tok++);
	result_buf->p_proto = atoi(*(tok++));
	result_buf->p_aliases = tok;
	*result = result_buf;
	ret = 0;
 DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
 DONE_NOUNLOCK:
	errno = ret;
	return errno;
}
libc_hidden_def(getprotoent_r)

static void __initbuf(void)
{
	if (!protobuf) {
		protobuf = malloc(SBUFSIZE);
		if (!protobuf)
			abort();
	}
}

struct protoent *getprotoent(void)
{
	struct protoent *result;

	__initbuf();
	getprotoent_r(&protoe, protobuf, SBUFSIZE, &result);
	return result;
}

int getprotobyname_r(const char *name,
					struct protoent *result_buf, char *buf, size_t buflen,
					struct protoent **result)
{
	register char **cp;
	int ret;

	__UCLIBC_MUTEX_LOCK(mylock);
	setprotoent(proto_stayopen);
	while (!(ret = getprotoent_r(result_buf, buf, buflen, result))) {
		if (strcmp(name, result_buf->p_name) == 0)
			break;
		for (cp = result_buf->p_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
	}
 gotname:
	if (!proto_stayopen)
		endprotoent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getprotobyname_r)

struct protoent *getprotobyname(const char *name)
{
	struct protoent *result;

	__initbuf();
	getprotobyname_r(name, &protoe, protobuf, SBUFSIZE, &result);
	return result;
}

int getprotobynumber_r(int proto,
					struct protoent *result_buf, char *buf,
					size_t buflen, struct protoent **result)
{
	int ret;

	__UCLIBC_MUTEX_LOCK(mylock);
	setprotoent(proto_stayopen);
	while (!(ret = getprotoent_r(result_buf, buf, buflen, result))) {
		if (proto == result_buf->p_proto)
			break;
	}
	if (!proto_stayopen)
		endprotoent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getprotobynumber_r)

struct protoent *getprotobynumber(int proto)
{
	struct protoent *result;

	__initbuf();
	getprotobynumber_r(proto, &protoe, protobuf, SBUFSIZE, &result);
	return result;
}

