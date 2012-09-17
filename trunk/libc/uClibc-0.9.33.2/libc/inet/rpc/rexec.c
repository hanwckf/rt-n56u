/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define __FORCE_GLIBC
#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <alloca.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#define SA_LEN(_x)      __libc_sa_len((_x)->sa_family)
extern int __libc_sa_len(sa_family_t __af) __THROW attribute_hidden;

/* int rexecoptions; - google does not know it */
static char *ahostbuf = NULL;

int
rexec_af(char **ahost, int rport, const char *name, const char *pass, const char *cmd, int *fd2p, sa_family_t af)
{
	struct sockaddr_storage sa2, from;
	struct addrinfo hints, *res0;
	const char *orig_name = name;
	const char *orig_pass = pass;
	u_short port = 0;
	int s, timo = 1, s3;
	char c;
	int gai;
	char servbuff[NI_MAXSERV];

	if (sizeof(servbuff) < sizeof(int)*3 + 2) {
		snprintf(servbuff, sizeof(servbuff), "%d", ntohs(rport));
		servbuff[sizeof(servbuff) - 1] = '\0';
	} else {
		sprintf(servbuff, "%d", ntohs(rport));
	}

	memset(&hints, '\0', sizeof(hints));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;
	gai = getaddrinfo(*ahost, servbuff, &hints, &res0);
	if (gai) {
		/* XXX: set errno? */
		return -1;
	}

	if (res0->ai_canonname) {
		if (!ahostbuf)
			ahostbuf = __uc_malloc(NI_MAXHOST);
		strncpy(ahostbuf, res0->ai_canonname, NI_MAXHOST);
		ahostbuf[NI_MAXHOST-1] = '\0';
		*ahost = ahostbuf;
	}
	else {
		*ahost = NULL;
		__set_errno(ENOENT);
		return -1;
	}
	ruserpass(res0->ai_canonname, &name, &pass);
retry:
	s = socket(res0->ai_family, res0->ai_socktype, 0);
	if (s < 0) {
		perror("rexec: socket");
		return -1;
	}
	if (connect(s, res0->ai_addr, res0->ai_addrlen) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(res0->ai_canonname);
		return -1;
	}
	if (fd2p == 0) {
		(void) write(s, "", 1);
		port = 0;
	} else {
		char num[32];
		int s2;
		socklen_t sa2len;

		s2 = socket(res0->ai_family, res0->ai_socktype, 0);
		if (s2 < 0) {
			(void) close(s);
			return -1;
		}
		listen(s2, 1);
		sa2len = sizeof(sa2);
		if (getsockname(s2, (struct sockaddr *)&sa2, &sa2len) < 0) {
			perror("getsockname");
			(void) close(s2);
			goto bad;
		} else if (sa2len != SA_LEN((struct sockaddr *)&sa2)) {
			__set_errno(EINVAL);
			(void) close(s2);
			goto bad;
		}
		port = 0;
		if (!getnameinfo((struct sockaddr *)&sa2, sa2len,
				 NULL, 0, servbuff, sizeof(servbuff),
				 NI_NUMERICSERV))
			port = atoi(servbuff);
		(void) sprintf(num, "%u", port);
		(void) write(s, num, strlen(num)+1);
		{
			socklen_t len = sizeof(from);
			s3 = TEMP_FAILURE_RETRY(accept(s2,
					(struct sockaddr *)&from, &len));
			close(s2);
			if (s3 < 0) {
				perror("accept");
				port = 0;
				goto bad;
			}
		}
		*fd2p = s3;
	}
	(void) write(s, name, strlen(name) + 1);
	/* should public key encypt the password here */
	(void) write(s, pass, strlen(pass) + 1);
	(void) write(s, cmd, strlen(cmd) + 1);

	/* We don't need the memory allocated for the name and the password
	   in ruserpass anymore.  */
	if (name != orig_name)
		free((char *) name);
	if (pass != orig_pass)
		free((char *) pass);

	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	freeaddrinfo(res0);
	return s;
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	freeaddrinfo(res0);
	return -1;
}
libc_hidden_def(rexec_af)

int
rexec(char **ahost, int rport, const char *name, const char *pass,
	  const char *cmd, int *fd2p)
{
	return rexec_af(ahost, rport, name, pass, cmd, fd2p, AF_INET);
}
