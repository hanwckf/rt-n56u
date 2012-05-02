/*
** protocols.c                           /etc/protocols access functions
**
** This file is part of the NYS Library.
**
** The NYS Library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
**
** The NYS Library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with the NYS Library; see the file COPYING.LIB.  If
** not, write to the Free Software Foundation, Inc., 675 Mass Ave,
** Cambridge, MA 02139, USA.
**
**
** Copyright (c) 1983 Regents of the University of California.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. All advertising materials mentioning features or use of this software
**    must display the following acknowledgement:
**	This product includes software developed by the University of
**	California, Berkeley and its contributors.
** 4. Neither the name of the University nor the names of its contributors
**    may be used to endorse or promote products derived from this software
**    without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#define __FORCE_GLIBC
#define _GNU_SOURCE
#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP);

#define	MAXALIASES	35
#define	SBUFSIZE	(BUFSIZ + 1 + (sizeof(char *) * MAXALIASES))

static FILE *protof = NULL;
static struct protoent proto;
static char *static_aliases = NULL;
static int proto_stayopen;

static void __initbuf(void)
{
    if (!static_aliases) {
	static_aliases = malloc(SBUFSIZE);
	if (!static_aliases)
	    abort();
    }
}

void setprotoent(int f)
{
    __UCLIBC_MUTEX_LOCK(mylock);
    if (protof == NULL)
	protof = fopen(_PATH_PROTOCOLS, "r" );
    else
	rewind(protof);
    proto_stayopen |= f;
    __UCLIBC_MUTEX_UNLOCK(mylock);
}

void endprotoent(void)
{
    __UCLIBC_MUTEX_LOCK(mylock);
    if (protof) {
	fclose(protof);
	protof = NULL;
    }
    proto_stayopen = 0;
    __UCLIBC_MUTEX_UNLOCK(mylock);
}

int getprotoent_r(struct protoent *result_buf,
		  char *buf, size_t buflen,
		  struct protoent **result)
{
    char *p;
    register char *cp, **q;
    char **proto_aliases;
    char *line;
	int rv;

    *result = NULL;

    if (buflen < sizeof(*proto_aliases)*MAXALIASES) {
	errno=ERANGE;
	return errno;
    }

    __UCLIBC_MUTEX_LOCK(mylock);
    proto_aliases=(char **)buf;
    buf+=sizeof(*proto_aliases)*MAXALIASES;
    buflen-=sizeof(*proto_aliases)*MAXALIASES;

    if (buflen < BUFSIZ+1) {
		errno=rv=ERANGE;
		goto DONE;
    }
    line=buf;
    buf+=BUFSIZ+1;
    buflen-=BUFSIZ+1;

    if (protof == NULL && (protof = fopen(_PATH_PROTOCOLS, "r" )) == NULL) {
		rv=errno;
		goto DONE;
    }
again:
    if ((p = fgets(line, BUFSIZ, protof)) == NULL) {
		rv=TRY_AGAIN;
		goto DONE;
    }

    if (*p == '#')
	goto again;
    cp = strpbrk(p, "#\n");
    if (cp == NULL)
	goto again;
    *cp = '\0';
    result_buf->p_name = p;
    cp = strpbrk(p, " \t");
    if (cp == NULL)
	goto again;
    *cp++ = '\0';
    while (*cp == ' ' || *cp == '\t')
	cp++;
    p = strpbrk(cp, " \t");
    if (p != NULL)
	*p++ = '\0';
    result_buf->p_proto = atoi(cp);
    q = result_buf->p_aliases = proto_aliases;
    if (p != NULL) {
	cp = p;
	while (cp && *cp) {
	    if (*cp == ' ' || *cp == '\t') {
		cp++;
		continue;
	    }
	    if (q < &proto_aliases[MAXALIASES - 1])
		*q++ = cp;
	    cp = strpbrk(cp, " \t");
	    if (cp != NULL)
		*cp++ = '\0';
	}
    }
    *q = NULL;
    *result=result_buf;

	rv = 0;

 DONE:
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return rv;
}

struct protoent * getprotoent(void)
{
    struct protoent *result;

    __initbuf();
    getprotoent_r(&proto, static_aliases, SBUFSIZE, &result);
    return result;
}


int getprotobyname_r(const char *name,
		    struct protoent *result_buf,
		    char *buf, size_t buflen,
		    struct protoent **result)
{
    register char **cp;
    int ret;

    __UCLIBC_MUTEX_LOCK(mylock);
    setprotoent(proto_stayopen);
    while (!(ret=getprotoent_r(result_buf, buf, buflen, result))) {
	if (strcmp(result_buf->p_name, name) == 0)
	    break;
	for (cp = result_buf->p_aliases; *cp != 0; cp++)
	    if (strcmp(*cp, name) == 0)
		goto found;
    }
found:
    if (!proto_stayopen)
	endprotoent();
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return *result?0:ret;
}


struct protoent * getprotobyname(const char *name)
{
    struct protoent *result;

    __initbuf();
    getprotobyname_r(name, &proto, static_aliases, SBUFSIZE, &result);
    return result;
}


int getprotobynumber_r (int proto_num,
			struct protoent *result_buf,
			char *buf, size_t buflen,
			struct protoent **result)
{
    int ret;

    __UCLIBC_MUTEX_LOCK(mylock);
    setprotoent(proto_stayopen);
    while (!(ret=getprotoent_r(result_buf, buf, buflen, result)))
	if (result_buf->p_proto == proto_num)
	    break;
    if (!proto_stayopen)
	endprotoent();
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return *result?0:ret;
}

struct protoent * getprotobynumber(int proto_num)
{
    struct protoent *result;

    __initbuf();
    getprotobynumber_r(proto_num, &proto, static_aliases,
                       SBUFSIZE, &result);
    return result;
}

