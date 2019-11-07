/*
 * Copyright (C) 2010 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/* /etc/networks
#   network-name  number     [aliases ...]
loopback          127.0.0.0  # optional aliases

network-name: symbolic name of the netwkork
number: official number of the network in dotted quad
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
#define	MAXALIASES	8
#define MAXTOKENS	(MINTOKENS + MAXALIASES + 1)
#define BUFSZ		(255) /* one line */
#define SBUFSIZE	(BUFSZ + 1 + (sizeof(char *) * MAXTOKENS))

static parser_t *netp = NULL;
static struct netent nete;
static char *netbuf = NULL;
static smallint net_stayopen;

void setnetent(int stayopen)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (netp)
		config_close(netp);
	netp = config_open(_PATH_NETWORKS);
	if (stayopen)
		net_stayopen = 1;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(setnetent)

void endnetent(void)
{
	__UCLIBC_MUTEX_LOCK(mylock);
	if (netp) {
		config_close(netp);
		netp = NULL;
	}
	net_stayopen = 0;
	__UCLIBC_MUTEX_UNLOCK(mylock);
}
libc_hidden_def(endnetent)

int getnetent_r(struct netent *result_buf,
				char *buf, size_t buflen, struct netent **result,
				int *h_errnop
				 )
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
	if (netp == NULL)
		setnetent(net_stayopen);
	if (netp == NULL)
		goto DONE;
	netp->data = buf;
	netp->data_len = aliaslen;
	netp->line_len = buflen - aliaslen;
	/* <name>[[:space:]]<netnumber>[[:space:]][<aliases>] */
	if (!config_read(netp, &tok, MAXTOKENS-1, MINTOKENS, "# \t/", PARSE_NORMAL)) {
		goto DONE;
	}
	result_buf->n_name = *(tok++);
	{
		struct addrinfo hints, *addri;
# define sa4_to_uint32(sa) \
	(ntohl(((struct sockaddr_in*)sa)->sin_addr.s_addr))
#ifdef __UCLIBC_HAS_IPV6__
# define sa6_to_uint8(sa) \
	(ntohl(((struct sockaddr_in6*)sa)->sin6_addr.s6_addr))
#endif
		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags = AI_NUMERICHOST;
		getaddrinfo(*(tok++), NULL, &hints, &addri);
		result_buf->n_addrtype = addri->ai_family;
		result_buf->n_net =
#if 0 /*FIXME: implement me! def __UCLIBC_HAS_IPV6__ */
			addri->ai_family == AF_INET6 ? sa6_to_uint8(addri->ai_addr) :
#endif
			sa4_to_uint32(addri->ai_addr);
		freeaddrinfo(addri);
	}
	result_buf->n_aliases = tok;
	*result = result_buf;
	ret = 0;
 DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
 DONE_NOUNLOCK:
	errno = ret;
	return errno;
}
libc_hidden_def(getnetent_r)

static void __initbuf(void)
{
	if (!netbuf) {
		netbuf = malloc(SBUFSIZE);
		if (!netbuf)
			abort();
	}
}

struct netent *getnetent(void)
{
	struct netent *result;
	int herrnop;

	__initbuf();
	getnetent_r(&nete, netbuf, SBUFSIZE, &result, &herrnop);
	return result;
}

int getnetbyname_r(const char *name,
					struct netent *result_buf, char *buf, size_t buflen,
					struct netent **result,
					int *h_errnop
					)
{
	register char **cp;
	int ret, herrnop;

	__UCLIBC_MUTEX_LOCK(mylock);
	setnetent(net_stayopen);
	while (!(ret = getnetent_r(result_buf, buf, buflen, result, &herrnop))) {
		if (strcmp(name, result_buf->n_name) == 0)
			break;
		for (cp = result_buf->n_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
	}
 gotname:
	if (!net_stayopen)
		endnetent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getnetbyname_r)

struct netent *getnetbyname(const char *name)
{
	struct netent *result;
	int herrnop;

	__initbuf();
	getnetbyname_r(name, &nete, netbuf, SBUFSIZE, &result, &herrnop);
	return result;
}

int getnetbyaddr_r(uint32_t net, int type,
					struct netent *result_buf, char *buf,
					size_t buflen, struct netent **result,
					int *h_errnop)
{
	int ret, herrnop;

	__UCLIBC_MUTEX_LOCK(mylock);
	setnetent(net_stayopen);
	while (!(ret = getnetent_r(result_buf, buf, buflen, result, &herrnop))) {
		if (net == result_buf->n_net && type == result_buf->n_addrtype)
			break;
	}
	if (!net_stayopen)
		endnetent();
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return *result ? 0 : ret;
}
libc_hidden_def(getnetbyaddr_r)

struct netent *getnetbyaddr(uint32_t net, int type)
{
	struct netent *result;
	int herrnop;

	__initbuf();
	getnetbyaddr_r(net, type, &nete, netbuf, SBUFSIZE, &result, &herrnop);
	return result;
}

