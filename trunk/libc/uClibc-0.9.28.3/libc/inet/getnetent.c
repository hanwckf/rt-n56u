/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

#define	MAXALIASES	35
static const char NETDB[] = _PATH_NETWORKS;
static FILE *netf = NULL;
static char *line = NULL;
static struct netent net;
static char *net_aliases[MAXALIASES];

int _net_stayopen;

void setnetent(int f)
{
    __UCLIBC_MUTEX_LOCK(mylock);
    if (netf == NULL)
	netf = fopen(NETDB, "r" );
    else
	rewind(netf);
    _net_stayopen |= f;
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return;
}

void endnetent(void)
{
    __UCLIBC_MUTEX_LOCK(mylock);
    if (netf) {
	fclose(netf);
	netf = NULL;
    }
    _net_stayopen = 0;
    __UCLIBC_MUTEX_UNLOCK(mylock);
}

static char * any(register char *cp, char *match)
{
    register char *mp, c;

    while ((c = *cp)) {
	for (mp = match; *mp; mp++)
	    if (*mp == c)
		return (cp);
	cp++;
    }
    return ((char *)0);
}

struct netent * getnetent(void)
{
    char *p;
    register char *cp, **q;
	struct netent *rv = NULL;

    __UCLIBC_MUTEX_LOCK(mylock);
    if (netf == NULL && (netf = fopen(NETDB, "r" )) == NULL) {
		goto DONE;
    }
again:

    if (!line) {
	line = malloc(BUFSIZ + 1);
	if (!line)
	    abort();
    }

    p = fgets(line, BUFSIZ, netf);
    if (p == NULL) {
		goto DONE;
    }
    if (*p == '#')
	goto again;
    cp = any(p, "#\n");
    if (cp == NULL)
	goto again;
    *cp = '\0';
    net.n_name = p;
    cp = any(p, " \t");
    if (cp == NULL)
	goto again;
    *cp++ = '\0';
    while (*cp == ' ' || *cp == '\t')
	cp++;
    p = any(cp, " \t");
    if (p != NULL)
	*p++ = '\0';
    net.n_net = inet_network(cp);
    net.n_addrtype = AF_INET;
    q = net.n_aliases = net_aliases;
    if (p != NULL)
	cp = p;
    while (cp && *cp) {
	if (*cp == ' ' || *cp == '\t') {
	    cp++;
	    continue;
	}
	if (q < &net_aliases[MAXALIASES - 1])
	    *q++ = cp;
	cp = any(cp, " \t");
	if (cp != NULL)
	    *cp++ = '\0';
    }
    *q = NULL;

	rv = &net;

 DONE:
    __UCLIBC_MUTEX_UNLOCK(mylock);
    return rv;
}

