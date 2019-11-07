/*
 * Copyright 1996 by Craig Metz
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 * Portions from the GNU C library,
 * Copyright (C) 2003, 2006 Free Software Foundation, Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* $USAGI: getaddrinfo.c,v 1.16 2001/10/04 09:52:03 sekiya Exp $ */

/* The Inner Net License, Version 2.00

  The author(s) grant permission for redistribution and use in source and
binary forms, with or without modification, of the software and documentation
provided that the following conditions are met:

0. If you receive a version of the software that is specifically labelled
   as not being for redistribution (check the version message and/or README),
   you are not permitted to redistribute that version of the software in any
   way or form.
1. All terms of the all other applicable copyrights and licenses must be
   followed.
2. Redistributions of source code must retain the authors' copyright
   notice(s), this list of conditions, and the following disclaimer.
3. Redistributions in binary form must reproduce the authors' copyright
   notice(s), this list of conditions, and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
4. All advertising materials mentioning features or use of this software
   must display the following acknowledgement with the name(s) of the
   authors as specified in the copyright notice(s) substituted where
   indicated:

	This product includes software developed by <name(s)>, The Inner
	Net, and other contributors.

5. Neither the name(s) of the author(s) nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ITS AUTHORS AND CONTRIBUTORS ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  If these license terms cause you a real problem, contact the author.  */

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#ifdef __UCLIBC_HAS_TLS__
#include <tls.h>
#endif
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "internal/parse_config.h"

#define GAIH_OKIFUNSPEC 0x0100
#define GAIH_EAI        ~(GAIH_OKIFUNSPEC)

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX  108
#endif

/* Useful for having small structure members/global variables */
typedef int8_t socktype_t;
typedef int8_t family_t;
typedef int8_t protocol_t;
struct BUG_too_small {
	char BUG_socktype_t_too_small[(0
			| SOCK_STREAM
			| SOCK_DGRAM
			| SOCK_RAW
			) <= 127 ? 1 : -1];
	char BUG_family_t_too_small[(0
			| AF_UNSPEC
			| AF_INET
			| AF_INET6
			) <= 127 ? 1 : -1];
	char BUG_protocol_t_too_small[(0
			| IPPROTO_TCP
			| IPPROTO_UDP
			) <= 127 ? 1 : -1];
};

struct gaih_service {
	const char *name;
	int num;
};

struct gaih_servtuple {
	struct gaih_servtuple *next;
	int socktype;
	int protocol;
	int port;
};

struct gaih_addrtuple {
	struct gaih_addrtuple *next;
	int family;
	char addr[16];
	uint32_t scopeid;
};

struct gaih_typeproto {
	socktype_t socktype;
	protocol_t protocol;
	int8_t protoflag;
	char name[4];
};
/* Values for `protoflag'.  */
#define GAI_PROTO_NOSERVICE 1
#define GAI_PROTO_PROTOANY  2

static const struct gaih_typeproto gaih_inet_typeproto[] = {
	{ 0          , 0          , 0, ""    },
	{ SOCK_STREAM, IPPROTO_TCP, 0, "tcp" },
	{ SOCK_DGRAM , IPPROTO_UDP, 0, "udp" },
	{ SOCK_RAW   , 0          , GAI_PROTO_PROTOANY|GAI_PROTO_NOSERVICE, "raw" },
	{ 0          , 0          , 0, ""    },
};

struct gaih {
	int family;
	int (*gaih)(const char *name, const struct gaih_service *service,
			const struct addrinfo *req, struct addrinfo **pai);
};

#define SEEN_IPV4 1
#define SEEN_IPV6 2

static unsigned __check_pf(void)
{
	unsigned seen = 0;

#if defined __UCLIBC_SUPPORT_AI_ADDRCONFIG__

	struct ifaddrs *ifa;
	struct ifaddrs *runp;

	/* Get the interface list via getifaddrs.  */
	if (getifaddrs(&ifa) != 0) {
		/* We cannot determine what interfaces are available.
		 * Be optimistic.  */
#if defined __UCLIBC_HAS_IPV4__
		seen |= SEEN_IPV4;
#endif
#if defined __UCLIBC_HAS_IPV6__
		seen |= SEEN_IPV6;
#endif
		return seen;
	}

	for (runp = ifa; runp != NULL; runp = runp->ifa_next) {
		if (runp->ifa_addr == NULL)
			continue;
#if defined __UCLIBC_HAS_IPV4__
		if (runp->ifa_addr->sa_family == PF_INET)
			seen |= SEEN_IPV4;
#endif
#if defined __UCLIBC_HAS_IPV6__
		if (runp->ifa_addr->sa_family == PF_INET6)
			seen |= SEEN_IPV6;
#endif
	}
	freeifaddrs(ifa);

#else

	/* AI_ADDRCONFIG is disabled, assume both ipv4 and ipv6 available. */
#if defined __UCLIBC_HAS_IPV4__
	seen |= SEEN_IPV4;
#endif
#if defined __UCLIBC_HAS_IPV6__
	seen |= SEEN_IPV6;
#endif

#endif /* __UCLIBC_SUPPORT_AI_ADDRCONFIG__ */

	return seen;
}

static int addrconfig(sa_family_t af)
{
	int s;
	int ret;
	int saved_errno = errno;
	unsigned seen;

	seen = __check_pf();
#if defined __UCLIBC_HAS_IPV4__
	if (af == AF_INET)
		ret = seen & SEEN_IPV4;
	else
#endif
#if defined __UCLIBC_HAS_IPV6__
	if (af == AF_INET6)
		ret = seen & SEEN_IPV6;
	else
#endif
	{
		s = socket(af, SOCK_DGRAM, 0);
		ret = 1; /* Assume PF_UNIX. */
		if (s < 0) {
			if (errno != EMFILE)
	        		ret = 0;
		} else
			close(s);
	}
	__set_errno(saved_errno);
	return ret;
}

#if 0
/* Using Unix sockets this way is a security risk.  */
static int
gaih_local(const char *name, const struct gaih_service *service,
		const struct addrinfo *req, struct addrinfo **pai)
{
	struct utsname utsname;
	struct addrinfo *ai = *pai;

	if ((name != NULL) && (req->ai_flags & AI_NUMERICHOST))
		return (GAIH_OKIFUNSPEC | -EAI_NONAME);

	if ((name != NULL) || (req->ai_flags & AI_CANONNAME))
		if (uname(&utsname) < 0)
			return -EAI_SYSTEM;

	if (name != NULL) {
		if (strcmp(name, "localhost") &&
		    strcmp(name, "local") &&
		    strcmp(name, "unix") &&
		    strcmp(name, utsname.nodename))
			return (GAIH_OKIFUNSPEC | -EAI_NONAME);
	}

	if (req->ai_protocol || req->ai_socktype) {
		const struct gaih_typeproto *tp = gaih_inet_typeproto + 1;

		while (tp->name[0]
		    && ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0
		       || (req->ai_socktype != 0 && req->ai_socktype != tp->socktype)
		       || (req->ai_protocol != 0 && !(tp->protoflag & GAI_PROTO_PROTOANY) && req->ai_protocol != tp->protocol))
		) {
			++tp;
		}
		if (! tp->name[0]) {
			if (req->ai_socktype)
				return (GAIH_OKIFUNSPEC | -EAI_SOCKTYPE);
			return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
		}
	}

	*pai = ai = malloc(sizeof(struct addrinfo) + sizeof(struct sockaddr_un)
			+ ((req->ai_flags & AI_CANONNAME)
			? (strlen(utsname.nodename) + 1) : 0));
	if (ai == NULL)
		return -EAI_MEMORY;

	ai->ai_next = NULL;
	ai->ai_flags = req->ai_flags;
	ai->ai_family = AF_LOCAL;
	ai->ai_socktype = req->ai_socktype ? req->ai_socktype : SOCK_STREAM;
	ai->ai_protocol = req->ai_protocol;
	ai->ai_addrlen = sizeof(struct sockaddr_un);
	ai->ai_addr = (void *)ai + sizeof(struct addrinfo);
#if 0 /* SALEN */
	((struct sockaddr_un *)ai->ai_addr)->sun_len = sizeof(struct sockaddr_un);
#endif /* SALEN */

	((struct sockaddr_un *)ai->ai_addr)->sun_family = AF_LOCAL;
	memset(((struct sockaddr_un *)ai->ai_addr)->sun_path, 0, UNIX_PATH_MAX);

	if (service) {
		struct sockaddr_un *sunp = (struct sockaddr_un *)ai->ai_addr;

		if (strchr(service->name, '/') != NULL) {
			if (strlen(service->name) >= sizeof(sunp->sun_path))
				return GAIH_OKIFUNSPEC | -EAI_SERVICE;
			strcpy(sunp->sun_path, service->name);
		} else {
			if (strlen(P_tmpdir "/") + 1 + strlen(service->name) >= sizeof(sunp->sun_path))
				return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
			stpcpy(stpcpy(sunp->sun_path, P_tmpdir "/"), service->name);
		}
	} else {
		/* This is a dangerous use of the interface since there is a time
		   window between the test for the file and the actual creation
		   (done by the caller) in which a file with the same name could
		   be created.  */
		char *buf = ((struct sockaddr_un *)ai->ai_addr)->sun_path;

		if (__path_search(buf, L_tmpnam, NULL, NULL, 0) != 0
		 || __gen_tempname(buf, __GT_NOCREATE, 0, 0) != 0
		) {
			return -EAI_SYSTEM;
		}
	}

	ai->ai_canonname = NULL;
	if (req->ai_flags & AI_CANONNAME)
		ai->ai_canonname = strcpy((char *)(ai + 1) + sizeof(struct sockaddr_un),
				utsname.nodename);
	return 0;
}
#endif	/* 0 */

static int
gaih_inet_serv(const char *servicename, const struct gaih_typeproto *tp,
		const struct addrinfo *req, struct gaih_servtuple *st)
{
	struct servent *s;
	size_t tmpbuflen = 1024;
	struct servent ts;
	char *tmpbuf;
	int r;

	while (1) {
		tmpbuf = alloca(tmpbuflen);
		r = getservbyname_r(servicename, tp->name, &ts, tmpbuf, tmpbuflen, &s);
		if (r == 0 && s != NULL)
			break;
		if (r != ERANGE)
			return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
		tmpbuflen *= 2;
	}
	st->next = NULL;
	st->socktype = tp->socktype;
	st->protocol = ((tp->protoflag & GAI_PROTO_PROTOANY) ? req->ai_protocol : tp->protocol);
	st->port = s->s_port;
	return 0;
}

#if defined __UCLIBC_HAS_IPV6__
static uint8_t __gai_precedence = 0;	/* =1 - IPv6, IPv4
					   =2 - IPv4, IPv6 */
#endif

/* NB: also uses h,pat,rc,no_data variables */
#define gethosts(_family, _type)						\
{										\
	int i, herrno;								\
	size_t tmpbuflen;							\
	struct hostent th;							\
	char *tmpbuf;								\
										\
	tmpbuflen = 512;							\
	no_data = 0;								\
	do {									\
		tmpbuflen *= 2;							\
		tmpbuf = alloca(tmpbuflen);					\
		rc = gethostbyname2_r(name, _family, &th, tmpbuf,		\
				tmpbuflen, &h, &herrno);			\
	} while (rc == ERANGE && herrno == NETDB_INTERNAL);			\
	if (rc != 0) {								\
		if (herrno == NETDB_INTERNAL) {					\
			__set_h_errno(herrno);					\
			return -EAI_SYSTEM;					\
		}								\
		if (herrno == TRY_AGAIN)					\
			no_data = EAI_AGAIN;					\
		else								\
			no_data = (herrno == NO_DATA);				\
	} else if (h != NULL) {							\
		for (i = 0; h->h_addr_list[i]; i++) {				\
			if (*pat == NULL) {					\
				*pat = alloca(sizeof(struct gaih_addrtuple));	\
				(*pat)->scopeid = 0;				\
			}							\
			(*pat)->next = NULL;					\
			(*pat)->family = _family;				\
			memcpy((*pat)->addr, h->h_addr_list[i], sizeof(_type));	\
			pat = &((*pat)->next);					\
		}								\
		if (_family == AF_INET6 && i > 0) {				\
			got_ipv6 = true;					\
		}								\
	}									\
}

static int
gaih_inet(const char *name, const struct gaih_service *service,
		const struct addrinfo *req, struct addrinfo **pai)
{
	struct gaih_servtuple nullserv;

	const struct gaih_typeproto *tp;
	struct gaih_servtuple *st;
	struct gaih_addrtuple *at;
	int rc;
	bool got_ipv6 = false;
	int v4mapped = req->ai_family == PF_INET6 && (req->ai_flags & AI_V4MAPPED);
	unsigned seen = 0;
	if (req->ai_flags & AI_ADDRCONFIG) {
		/* "seen" is only used when AI_ADDRCONFIG is specified.
		   Avoid unnecessary call to __check_pf() otherwise
		   since it can be costly especially when RSBAC-Net is enabled.  */
		seen = __check_pf();
	}

	memset(&nullserv, 0, sizeof(nullserv));

	tp = gaih_inet_typeproto;
	if (req->ai_protocol || req->ai_socktype) {
		++tp;
		while (tp->name[0]) {
			if ((req->ai_socktype == 0 || req->ai_socktype == tp->socktype)
			 && (req->ai_protocol == 0 || req->ai_protocol == tp->protocol || (tp->protoflag & GAI_PROTO_PROTOANY))
			) {
				goto found;
			}
			++tp;
		}
		if (req->ai_socktype)
			return (GAIH_OKIFUNSPEC | -EAI_SOCKTYPE);
		return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
 found: ;
	}

	st = &nullserv;
	if (service != NULL) {
		if ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0)
			return (GAIH_OKIFUNSPEC | -EAI_SERVICE);

		if (service->num < 0) {
			if (tp->name[0]) {
				st = alloca(sizeof(struct gaih_servtuple));
				rc = gaih_inet_serv(service->name, tp, req, st);
				if (rc)
					return rc;
			} else {
				struct gaih_servtuple **pst = &st;
				for (tp++; tp->name[0]; tp++) {
					struct gaih_servtuple *newp;

					if ((tp->protoflag & GAI_PROTO_NOSERVICE) != 0)
						continue;

					if (req->ai_socktype != 0 && req->ai_socktype != tp->socktype)
						continue;
					if (req->ai_protocol != 0
					 && !(tp->protoflag & GAI_PROTO_PROTOANY)
					 && req->ai_protocol != tp->protocol)
						continue;

					newp = alloca(sizeof(struct gaih_servtuple));
					rc = gaih_inet_serv(service->name, tp, req, newp);
					if (rc) {
						if (rc & GAIH_OKIFUNSPEC)
							continue;
						return rc;
					}

					*pst = newp;
					pst = &(newp->next);
				}
				if (st == &nullserv)
					return (GAIH_OKIFUNSPEC | -EAI_SERVICE);
			}
		} else {
			st = alloca(sizeof(struct gaih_servtuple));
			st->next = NULL;
			st->socktype = tp->socktype;
			st->protocol = ((tp->protoflag & GAI_PROTO_PROTOANY)
					? req->ai_protocol : tp->protocol);
			st->port = htons(service->num);
		}
	} else if (req->ai_socktype || req->ai_protocol) {
		st = alloca(sizeof(struct gaih_servtuple));
		st->next = NULL;
		st->socktype = tp->socktype;
		st->protocol = ((tp->protoflag & GAI_PROTO_PROTOANY)
				? req->ai_protocol : tp->protocol);
		st->port = 0;
	} else {
		/*
		 * Neither socket type nor protocol is set.  Return all socket types
		 * we know about.
		 */
		struct gaih_servtuple **lastp = &st;
		for (++tp; tp->name[0]; ++tp) {
			struct gaih_servtuple *newp;

			newp = alloca(sizeof(struct gaih_servtuple));
			newp->next = NULL;
			newp->socktype = tp->socktype;
			newp->protocol = tp->protocol;
			newp->port = 0;

			*lastp = newp;
			lastp = &newp->next;
		}
	}

	at = NULL;
	if (name != NULL) {
		at = alloca(sizeof(struct gaih_addrtuple));
		at->family = AF_UNSPEC;
		at->scopeid = 0;
		at->next = NULL;

		if (inet_pton(AF_INET, name, at->addr) > 0) {
			if (req->ai_family != AF_UNSPEC && req->ai_family != AF_INET && !v4mapped)
				return -EAI_FAMILY;
			at->family = AF_INET;
		}

#if defined __UCLIBC_HAS_IPV6__
		if (at->family == AF_UNSPEC) {
			char *namebuf = strdupa(name);
			char *scope_delim;

			scope_delim = strchr(namebuf, SCOPE_DELIMITER);
			if (scope_delim != NULL)
				*scope_delim = '\0';

			if (inet_pton(AF_INET6, namebuf, at->addr) > 0) {
				if (req->ai_family != AF_UNSPEC && req->ai_family != AF_INET6)
					return -EAI_FAMILY;
				at->family = AF_INET6;
				if (scope_delim != NULL) {
					int try_numericscope = 0;
					uint32_t *a32 = (uint32_t*)at->addr;
					if (IN6_IS_ADDR_LINKLOCAL(a32) || IN6_IS_ADDR_MC_LINKLOCAL(at->addr)) {
						at->scopeid = if_nametoindex(scope_delim + 1);
						if (at->scopeid == 0)
							try_numericscope = 1;
					} else
						try_numericscope = 1;

					if (try_numericscope != 0) {
						char *end;
						assert(sizeof(uint32_t) <= sizeof(unsigned long));
						at->scopeid = (uint32_t)strtoul(scope_delim + 1, &end, 10);
						if (*end != '\0')
							return (GAIH_OKIFUNSPEC | -EAI_NONAME);
					}
				}
			}
		}
#endif

		if (at->family == AF_UNSPEC && !(req->ai_flags & AI_NUMERICHOST)) {
			struct hostent *h;
			struct gaih_addrtuple **pat = &at;
			int no_data, no_inet6_data;
#if defined __UCLIBC_HAS_IPV6__
			bool first_try = true;
#endif

			/*
			 * If we are looking for both IPv4 and IPv6 address we don't want
			 * the lookup functions to automatically promote IPv4 addresses to
			 * IPv6 addresses.
			 */
			no_inet6_data = no_data = 0;
#if defined __UCLIBC_HAS_IPV6__
			if (__gai_precedence == 2)
				goto try_v4;

 try_v6:
			if (req->ai_family == AF_UNSPEC || req->ai_family == AF_INET6)
				if (!(req->ai_flags & AI_ADDRCONFIG) || (seen & SEEN_IPV6))
					gethosts(AF_INET6, struct in6_addr);
			no_inet6_data = no_data;
			if (!first_try)
				goto tried_all;
			first_try = false;

 try_v4:
#endif
			if (req->ai_family == AF_INET
			 || (!v4mapped && req->ai_family == AF_UNSPEC)
			 || (v4mapped && (!got_ipv6 || (req->ai_flags & AI_ALL)))
			) {
				if (!(req->ai_flags & AI_ADDRCONFIG) || (seen & SEEN_IPV4))
					gethosts(AF_INET, struct in_addr);
			}
#if defined __UCLIBC_HAS_IPV6__
			if (first_try) {
				first_try = false;
				goto try_v6;
			}

 tried_all:
#endif
			if (no_data != 0 && no_inet6_data != 0) {
				/* If both requests timed out report this. */
				if (no_data == EAI_AGAIN && no_inet6_data == EAI_AGAIN)
					return -EAI_AGAIN;
				/*
				 * We made requests but they turned out no data.
				 * The name is known, though.
				 */
				return (GAIH_OKIFUNSPEC | -EAI_AGAIN);
			}
		}

		if (at->family == AF_UNSPEC)
			return (GAIH_OKIFUNSPEC | -EAI_NONAME);
	} else {
		struct gaih_addrtuple *atr;

		atr = at = alloca(sizeof(struct gaih_addrtuple));
		memset(at, '\0', sizeof(struct gaih_addrtuple));
		if (req->ai_family == 0) {
			at->next = alloca(sizeof(struct gaih_addrtuple));
			memset(at->next, '\0', sizeof(struct gaih_addrtuple));
		}
#if defined __UCLIBC_HAS_IPV6__
		if (req->ai_family == 0 || req->ai_family == AF_INET6) {
			at->family = AF_INET6;
			if ((req->ai_flags & AI_PASSIVE) == 0)
				memcpy(at->addr, &in6addr_loopback, sizeof(struct in6_addr));
			atr = at->next;
		}
#endif
		if (req->ai_family == 0 || req->ai_family == AF_INET) {
			atr->family = AF_INET;
			if ((req->ai_flags & AI_PASSIVE) == 0) {
				uint32_t *a = (uint32_t*)atr->addr;
				*a = htonl(INADDR_LOOPBACK);
			}
		}
	}

	if (pai == NULL)
		return 0;

	{
		const char *c = NULL;
		struct gaih_servtuple *st2;
		struct gaih_addrtuple *at2 = at;
		size_t socklen, namelen;
		sa_family_t family;

		/*
		 * buffer is the size of an unformatted IPv6 address in
		 * printable format.
		 */
		char buffer[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];

		while (at2 != NULL) {
			c = inet_ntop(at2->family, at2->addr, buffer, sizeof(buffer));
			if (c) {
				namelen = strlen(c) + 1;
			} else if (req->ai_flags & AI_CANONNAME) {
				struct hostent *h = NULL;
				int herrno;
				struct hostent th;
				size_t tmpbuflen = 512;
				char *tmpbuf;

				/* Hint says numeric, but address is not */
				if (req->ai_flags & AI_NUMERICHOST)
					return -EAI_NONAME;

				do {
					tmpbuflen *= 2;
					tmpbuf = alloca(tmpbuflen);
					rc = gethostbyaddr_r(at2->addr,
#ifdef __UCLIBC_HAS_IPV6__
						((at2->family == AF_INET6)
						 ? sizeof(struct in6_addr)
						 : sizeof(struct in_addr)),
#else
						sizeof(struct in_addr),
#endif
						at2->family,
						&th, tmpbuf, tmpbuflen,
						&h, &herrno);
				} while (rc == ERANGE && herrno == NETDB_INTERNAL);

				if (rc != 0 && herrno == NETDB_INTERNAL) {
					__set_h_errno(herrno);
					return -EAI_SYSTEM;
				}

				if (h != NULL)
					c = h->h_name;

				if (c == NULL)
					return (GAIH_OKIFUNSPEC | -EAI_NONAME);

				namelen = strlen(c) + 1;
			} else
				namelen = 0;

#if defined __UCLIBC_HAS_IPV6__
			if (at2->family == AF_INET6 || v4mapped) {
				family = AF_INET6;
				socklen = sizeof(struct sockaddr_in6);

				/* If we looked up IPv4 mapped address discard them here if
				   the caller isn't interested in all address and we have
				   found at least one IPv6 address.  */
				if (got_ipv6
				  && (req->ai_flags & (AI_V4MAPPED|AI_ALL)) == AI_V4MAPPED
				  && IN6_IS_ADDR_V4MAPPED (at2->addr))
				goto ignore;
			}
#endif
#if defined __UCLIBC_HAS_IPV4__ && defined __UCLIBC_HAS_IPV6__
			else
#endif
#if defined __UCLIBC_HAS_IPV4__
			{
				family = AF_INET;
				socklen = sizeof(struct sockaddr_in);
			}
#endif
			for (st2 = st; st2 != NULL; st2 = st2->next) {
				if (req->ai_flags & AI_ADDRCONFIG) {
					if (family == AF_INET && !(seen & SEEN_IPV4))
						break;
#if defined __UCLIBC_HAS_IPV6__
					else if (family == AF_INET6 && !(seen & SEEN_IPV6))
						break;
#endif
				}
				*pai = malloc(sizeof(struct addrinfo) + socklen + namelen);
				if (*pai == NULL)
					return -EAI_MEMORY;

				(*pai)->ai_flags = req->ai_flags;
				(*pai)->ai_family = family;
				(*pai)->ai_socktype = st2->socktype;
				(*pai)->ai_protocol = st2->protocol;
				(*pai)->ai_addrlen = socklen;
				(*pai)->ai_addr = (void *) (*pai) + sizeof(struct addrinfo);
#if 0 /* SALEN */
				(*pai)->ai_addr->sa_len = socklen;
#endif
				(*pai)->ai_addr->sa_family = family;

#if defined __UCLIBC_HAS_IPV6__
				if (family == AF_INET6)	{
					struct sockaddr_in6 *sin6p = (struct sockaddr_in6 *) (*pai)->ai_addr;

					sin6p->sin6_flowinfo = 0;
					if (at2->family == AF_INET6) {
						memcpy(&sin6p->sin6_addr,
							at2->addr, sizeof(struct in6_addr));
					} else {
						sin6p->sin6_addr.s6_addr32[0] = 0;
						sin6p->sin6_addr.s6_addr32[1] = 0;
						sin6p->sin6_addr.s6_addr32[2] = htonl(0x0000ffff);
						memcpy(&sin6p->sin6_addr.s6_addr32[3],
							at2->addr, sizeof(sin6p->sin6_addr.s6_addr32[3]));
					}
					sin6p->sin6_port = st2->port;
					sin6p->sin6_scope_id = at2->scopeid;
				}
#endif
#if defined __UCLIBC_HAS_IPV4__ && defined __UCLIBC_HAS_IPV6__
				else
#endif
#if defined __UCLIBC_HAS_IPV4__
				{
					struct sockaddr_in *sinp = (struct sockaddr_in *) (*pai)->ai_addr;

					memcpy(&sinp->sin_addr, at2->addr, sizeof(struct in_addr));
					sinp->sin_port = st2->port;
					memset(sinp->sin_zero, '\0', sizeof(sinp->sin_zero));
				}
#endif
				if (c) {
					(*pai)->ai_canonname = ((void *) (*pai) +
							sizeof(struct addrinfo) + socklen);
					strcpy((*pai)->ai_canonname, c);
				} else {
					(*pai)->ai_canonname = NULL;
				}
				(*pai)->ai_next = NULL;
				pai = &((*pai)->ai_next);
			}
ignore:
			at2 = at2->next;
		}
	}
	return 0;
}

static const struct gaih gaih[] = {
#if defined __UCLIBC_HAS_IPV6__
	{ PF_INET6, gaih_inet },
#endif
	{ PF_INET, gaih_inet },
#if 0
	{ PF_LOCAL, gaih_local },
#endif
	{ PF_UNSPEC, NULL }
};

#if defined __UCLIBC_HAS_IPV6__

/*
 * A call to getaddrinfo might return multiple answers. To provide
 * possibility to change the sorting we must use /etc/gai.conf file,
 * like glibc.
 *
 * gai.conf format:
 *
 * label <netmask> <precedence>
 *				The value is added to the label table used in
 *				the RFC 3484 sorting. If any label definition
 *				is present in the configuration file is present,
 *				the default table is not used. All the label
 *				definitions of the default table which are to
 *				be maintained have to be duplicated.
 * precedence <netmask> <precedence>
 * 				This keyword is similar to label, but instead
 *				the value is added to the precedence table as
 *				specified in RFC 3484. Once again, the presence
 *				of a single precedence line in the configuration
 *				file causes the default table to not be used.
 *
 * The simplified uclibc's implementation allows to change the IPv4/IPv6
 * sorting order for a whole address space only, i.e
 *  "precedence ::ffff:0:0/96 100" is only supported.
 */
static void __gai_conf_parse(void)
{
	/* NO reread of /etc/gai.conf on change. */
	if (__gai_precedence != 0)
		return;

	__gai_precedence = 1; /* default IPv6 */

	parser_t *parser;
	char **tok = NULL;

	parser = config_open("/etc/gai.conf");
	if (!parser)
		return;

	while (config_read(parser, &tok, 3, 3, "# \t", PARSE_NORMAL)) {
		if (strcmp(tok[0], "precedence") == 0) {
			char *pfx;
			struct in6_addr mask;

			pfx = strchr(tok[1], '/');
			if (!pfx)
				continue;
			*(pfx++) = 0;
			if (inet_pton(AF_INET6, tok[1], &mask) <= 0)
				continue;
			if (IN6_IS_ADDR_V4MAPPED(&mask)
			    && mask.s6_addr32[3] == 0
			    && atoi(pfx) == 96 && atoi(tok[2]) == 100)
				__gai_precedence = 2;	/* IPv4 first */
		}
	}
	config_close(parser);
}
#else
# define __gai_conf_parse(x)
#endif /* __UCLIBC_HAS_IPV6__ */

void
freeaddrinfo(struct addrinfo *ai)
{
	struct addrinfo *p;

	while (ai != NULL) {
		p = ai;
		ai = ai->ai_next;
		free(p);
	}
}
libc_hidden_def(freeaddrinfo)

int
getaddrinfo(const char *name, const char *service,
	     const struct addrinfo *hints, struct addrinfo **pai)
{
	int i, j, last_i;
	struct addrinfo *p, **end;
	const struct gaih *g, *pg;
	struct gaih_service gaih_service, *pservice;
	struct addrinfo default_hints;

	if (name != NULL && name[0] == '*' && name[1] == 0)
		name = NULL;

	if (service != NULL && service[0] == '*' && service[1] == 0)
		service = NULL;

	if (name == NULL && service == NULL)
		return EAI_NONAME;

	if (hints == NULL) {
		memset(&default_hints, 0, sizeof(default_hints));
		if (AF_UNSPEC != 0)
			default_hints.ai_family = AF_UNSPEC;
		hints = &default_hints;
	}

	if (hints->ai_flags & ~(AI_PASSIVE|AI_CANONNAME|AI_NUMERICHOST|
			AI_ADDRCONFIG|AI_V4MAPPED|AI_NUMERICSERV|AI_ALL))
		return EAI_BADFLAGS;

	if ((hints->ai_flags & AI_CANONNAME) && name == NULL)
		return EAI_BADFLAGS;

	if (service && service[0]) {
		char *c;
		gaih_service.name = service;
		gaih_service.num = strtoul(gaih_service.name, &c, 10);
		if (*c != '\0') {
			if (hints->ai_flags & AI_NUMERICSERV)
				return EAI_NONAME;
			gaih_service.num = -1;
		}
		pservice = &gaih_service;
	} else
		pservice = NULL;

	__gai_conf_parse();
	g = gaih;
	pg = NULL;
	p = NULL;
	end = NULL;
	if (pai)
		end = &p;
	i = 0;
	last_i = 0;
	j = 0;
	while (g->gaih) {
		if (hints->ai_family == g->family || hints->ai_family == AF_UNSPEC) {
			if ((hints->ai_flags & AI_ADDRCONFIG) && !addrconfig(g->family)) {
				++g;
				continue;
			}
			j++;
			if (pg == NULL || pg->gaih != g->gaih) {
				pg = g;
				i = g->gaih(name, pservice, hints, end);
				if (i != 0) {
					last_i = i;
					if (hints->ai_family == AF_UNSPEC && (i & GAIH_OKIFUNSPEC))
						continue;
					/*if (p) - freeaddrinfo works ok on NULL too */
						freeaddrinfo(p);
					return -(i & GAIH_EAI);
				}
				if (end)
					while (*end)
						end = &((*end)->ai_next);
			}
		}
		++g;
	}

	if (j == 0)
		return EAI_FAMILY;

	if (p) {
		*pai = p;
		return 0;
	}

	if (pai == NULL && last_i == 0)
		return 0;

	/* if (p) - never happens, see above */
	/*	freeaddrinfo(p); */

	return last_i ? -(last_i & GAIH_EAI) : EAI_NONAME;
}
libc_hidden_def(getaddrinfo)
