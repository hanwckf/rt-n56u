/*
 * Copyright (c) 1983, 1993, 1994
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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

#if 0
static char sccsid[] = "@(#)rcmd.c	8.3 (Berkeley) 3/26/94";
#endif /* LIBC_SCCS and not lint */

#define __FORCE_GLIBC
#include <features.h>

#define __USE_GNU
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __UCLIBC_HAS_THREADS__
#undef __UCLIBC_HAS_THREADS__
#warning FIXME I am not reentrant yet...
#endif


/* some forward declarations */
static int __ivaliduser2(FILE *hostf, u_int32_t raddr,
			 const char *luser, const char *ruser, const char *rhost);
static int iruserok2 (u_int32_t raddr, int superuser, const char *ruser, 
		      const char *luser, const char *rhost);


int rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
     char **ahost;
     u_short rport;
     const char *locuser, *remuser, *cmd;
     int *fd2p;
{
#ifdef __UCLIBC_HAS_THREADS__
	int herr;
        struct hostent hostbuf;
	size_t hstbuflen;
	char *tmphstbuf;
#endif
        struct hostent *hp;
	struct sockaddr_in sin, from;
	struct pollfd pfd[2];
	int32_t oldmask;
	pid_t pid;
	int s, lport, timo;
	char c;

	pid = getpid();

#ifdef __UCLIBC_HAS_THREADS__
	hstbuflen = 1024;
#ifdef __ARCH_HAS_MMU__
	tmphstbuf = alloca (hstbuflen);
#else
	tmphstbuf = malloc (hstbuflen);
#endif

	while (gethostbyname_r (*ahost, &hostbuf, tmphstbuf, 
		    hstbuflen, &hp, &herr) != 0 || hp == NULL)
	{
	    if (herr != NETDB_INTERNAL || errno != ERANGE)
	    {
		__set_h_errno (herr);
#ifndef __ARCH_HAS_MMU__
		free(tmphstbuf);
#endif
		herror(*ahost);
		return -1;
	    }
	    else
	    {
		/* Enlarge the buffer.  */
		hstbuflen *= 2;
#ifdef __ARCH_HAS_MMU__
		tmphstbuf = alloca (hstbuflen);
#else
		if (tmphstbuf) {
		    free(tmphstbuf);
		}
		tmphstbuf = malloc (hstbuflen);
#endif
	    }
	}
#ifndef __ARCH_HAS_MMU__
	free(tmphstbuf);
#endif
#else /* call the non-reentrant version */
	if ((hp = gethostbyname(*ahost)) == NULL) {
	    return -1;
	}
#endif
	pfd[0].events = POLLIN;
	pfd[1].events = POLLIN;
	
        *ahost = hp->h_name;
        oldmask = sigblock(sigmask(SIGURG)); /* __sigblock */
	for (timo = 1, lport = IPPORT_RESERVED - 1;;) {
		s = rresvport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
			    (void)fprintf(stderr,
					  "rcmd: socket: All ports in use\n");
			else
			    (void)fprintf(stderr, "rcmd: socket: %m\n");
			sigsetmask(oldmask); /* sigsetmask */
			return -1;
		}
		fcntl(s, F_SETOWN, pid); /* __fcntl */
		sin.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0], &sin.sin_addr,
		      MIN (sizeof (sin.sin_addr), hp->h_length));
		sin.sin_port = rport;
		if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0) /* __connect */
			break;
		(void)close(s); /* __close */
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
       		        (void)sleep(timo); /* __sleep */
			timo *= 2;
			continue;
		}
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			(void)fprintf(stderr, "connect to address %s: ",
			    inet_ntoa(sin.sin_addr));
			__set_errno (oerrno);
			perror(0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0], &sin.sin_addr,
			      MIN (sizeof (sin.sin_addr), hp->h_length));
			(void)fprintf(stderr, "Trying %s...\n",
			    inet_ntoa(sin.sin_addr));
			continue;
		}
		(void)fprintf(stderr, "%s: %m\n", hp->h_name);
		sigsetmask(oldmask); /* __sigsetmask */
		return -1;
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1); /* __write */		    
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;
		socklen_t len = sizeof(from);

		if (s2 < 0)
			goto bad;
		listen(s2, 1);
		(void)snprintf(num, sizeof(num), "%d", lport); /* __snprintf */
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			(void)fprintf(stderr,
				      "rcmd: write (setting up stderr): %m\n");
			(void)close(s2); /* __close */
			goto bad;
		}
		pfd[0].fd = s;
		pfd[1].fd = s2;
		__set_errno (0);
		if (poll (pfd, 2, -1) < 1 || (pfd[1].revents & POLLIN) == 0){
		    if (errno != 0)
			(void)fprintf(stderr, "rcmd: poll (setting up stderr): %m\n");
		    else
			(void)fprintf(stderr, "poll: protocol failure in circuit setup\n");
			(void)close(s2);
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void)close(s2);
		if (s3 < 0) {
			(void)fprintf(stderr,
			    "rcmd: accept: %m\n");
			lport = 0;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED ||
		    from.sin_port < IPPORT_RESERVED / 2) {
			(void)fprintf(stderr,
			    "socket: protocol failure in circuit setup\n");
			goto bad2;
		}
	}
	(void)write(s, locuser, strlen(locuser)+1);
	(void)write(s, remuser, strlen(remuser)+1);
	(void)write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		(void)fprintf(stderr,
		    "rcmd: %s: %m\n", *ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void)write(STDERR_FILENO, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	sigsetmask(oldmask);
	return s;
bad2:
	if (lport)
		(void)close(*fd2p);
bad:
	(void)close(s);
	sigsetmask(oldmask);
	return -1;
}

int rresvport(int *alport)
{
    struct sockaddr_in sin;
    int s;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
	return -1;
    for (;;) {
	sin.sin_port = htons((u_short)*alport);
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
	    return s;
	if (errno != EADDRINUSE) {
	    (void)close(s);
	    return -1;
	}
	(*alport)--;
	if (*alport == IPPORT_RESERVED/2) {
	    (void)close(s);
	    __set_errno (EAGAIN);		/* close */
	    return -1;
	}
    }
    
    return -1;
}

int	__check_rhosts_file = 1;
char    *__rcmd_errstr;

int ruserok(rhost, superuser, ruser, luser)
	const char *rhost, *ruser, *luser;
	int superuser;
{
        struct hostent *hp;
	u_int32_t addr;
	char **ap;
#ifdef __UCLIBC_HAS_THREADS__
	size_t buflen;
	char *buffer;
	int herr;
	struct hostent hostbuf;
#endif

#ifdef __UCLIBC_HAS_THREADS__
	buflen = 1024;
#ifdef __ARCH_HAS_MMU__
	buffer = alloca (buflen);
#else
	buffer = malloc (buflen);
#endif

	while (gethostbyname_r (rhost, &hostbuf, buffer, 
		    buflen, &hp, &herr) != 0 || hp == NULL) 
	{
	    if (herr != NETDB_INTERNAL || errno != ERANGE) {
#ifndef __ARCH_HAS_MMU__
		free(buffer);
#endif
		return -1;
	    } else
	    {
		/* Enlarge the buffer.  */
		buflen *= 2;
#ifdef __ARCH_HAS_MMU__
		buffer = alloca (buflen);
#else
		if (buffer) {
		    free(buffer);
		}
		buffer = malloc (buflen);
#endif
	    }
	}
#ifndef __ARCH_HAS_MMU__
	free(buffer);
#endif
#else
	if ((hp = gethostbyname(rhost)) == NULL) {
		return -1;
	}
#endif
	for (ap = hp->h_addr_list; *ap; ++ap) {
		bcopy(*ap, &addr, sizeof(addr));
		if (iruserok2(addr, superuser, ruser, luser, rhost) == 0)
			return 0;
	}
	return -1;
}


/* Extremely paranoid file open function. */
static FILE *
iruserfopen (char *file, uid_t okuser)
{
  struct stat st;
  char *cp = NULL;
  FILE *res = NULL;

  /* If not a regular file, if owned by someone other than user or
     root, if writeable by anyone but the owner, or if hardlinked
     anywhere, quit.  */
  cp = NULL;
  if (lstat (file, &st))
    cp = "lstat failed";
  else if (!S_ISREG (st.st_mode))
    cp = "not regular file";
  else
    {
      res = fopen (file, "r");
      if (!res)
	cp = "cannot open";
      else if (fstat (fileno (res), &st) < 0)
	cp = "fstat failed";
      else if (st.st_uid && st.st_uid != okuser)
	cp = "bad owner";
      else if (st.st_mode & (S_IWGRP|S_IWOTH))
	cp = "writeable by other than owner";
      else if (st.st_nlink > 1)
	cp = "hard linked somewhere";
    }

  /* If there were any problems, quit.  */
  if (cp != NULL)
    {
      __rcmd_errstr = cp;
      if (res)
	fclose (res);
      return NULL;
    }

  return res;
}


/*
 * New .rhosts strategy: We are passed an ip address. We spin through
 * hosts.equiv and .rhosts looking for a match. When the .rhosts only
 * has ip addresses, we don't have to trust a nameserver.  When it
 * contains hostnames, we spin through the list of addresses the nameserver
 * gives us and look for a match.
 *
 * Returns 0 if ok, -1 if not ok.
 */
static int
iruserok2 (raddr, superuser, ruser, luser, rhost)
     u_int32_t raddr;
     int superuser;
     const char *ruser, *luser, *rhost;
{
	FILE *hostf = NULL;
	int isbad = -1;

	if (!superuser)
		hostf = iruserfopen (_PATH_HEQUIV, 0);
	
	if (hostf) {
		isbad = __ivaliduser2 (hostf, raddr, luser, ruser, rhost);
		fclose (hostf);

		if (!isbad)
			return 0;
	}

	if (__check_rhosts_file || superuser) {
		char *pbuf;
		struct passwd *pwd;
		size_t dirlen;
		uid_t uid;

#ifdef __UCLIBC_HAS_THREADS__
		size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
		struct passwd pwdbuf;
#ifdef __ARCH_HAS_MMU__
		char *buffer = alloca (buflen);
#else
		char *buffer = malloc (buflen);
#endif

		if (getpwnam_r (luser, &pwdbuf, buffer, 
			    buflen, &pwd) != 0 || pwd == NULL)
		{
#ifndef __ARCH_HAS_MMU__
			free(buffer);
#endif
			return -1;
		}
#ifndef __ARCH_HAS_MMU__
		free(buffer);
#endif
#else
		if ((pwd = getpwnam(luser)) == NULL)
			return -1;
#endif

		dirlen = strlen (pwd->pw_dir);
		pbuf = malloc (dirlen + sizeof "/.rhosts");
		strcpy (pbuf, pwd->pw_dir);
		strcat (pbuf, "/.rhosts");

		/* Change effective uid while reading .rhosts.  If root and
		   reading an NFS mounted file system, can't read files that
		   are protected read/write owner only.  */
		uid = geteuid ();
		seteuid (pwd->pw_uid);
		hostf = iruserfopen (pbuf, pwd->pw_uid);
		free(pbuf);
		
		if (hostf != NULL) {
			isbad = __ivaliduser2 (hostf, raddr, luser, ruser, rhost);
			fclose (hostf);
		}
		
		seteuid (uid);
		return isbad;
	}
	return -1;
}

/* This is the exported version.  */
int iruserok (u_int32_t raddr, int superuser, const char * ruser, const char * luser)
{
	return iruserok2 (raddr, superuser, ruser, luser, "-");
}


/*
 * XXX
 * Don't make static, used by lpd(8).
 *
 * This function is not used anymore. It is only present because lpd(8)
 * calls it (!?!). We simply call __invaliduser2() with an illegal rhost
 * argument. This means that netgroups won't work in .rhost/hosts.equiv
 * files. If you want lpd to work with netgroups, fix lpd to use ruserok()
 * or PAM.
 * Returns 0 if ok, -1 if not ok.
 */
int
__ivaliduser(FILE *hostf, u_int32_t raddr, const char *luser, const char *ruser)
{
	return __ivaliduser2(hostf, raddr, luser, ruser, "-");
}


/* Returns 1 on positive match, 0 on no match, -1 on negative match.  */
static int
__icheckhost (u_int32_t raddr, char *lhost, const char *rhost)
{
	struct hostent *hp;
	u_int32_t laddr;
	int negate=1;    /* Multiply return with this to get -1 instead of 1 */
	char **pp;

#ifdef __UCLIBC_HAS_THREADS__
	int save_errno;
	size_t buflen;
	char *buffer;
	struct hostent hostbuf;
	int herr;
#endif

#ifdef HAVE_NETGROUP
	/* Check nis netgroup.  */
	if (strncmp ("+@", lhost, 2) == 0)
		return innetgr (&lhost[2], rhost, NULL, NULL);

	if (strncmp ("-@", lhost, 2) == 0)
		return -innetgr (&lhost[2], rhost, NULL, NULL);
#endif /* HAVE_NETGROUP */

	/* -host */
	if (strncmp ("-", lhost,1) == 0) {
		negate = -1;
		lhost++;
	} else if (strcmp ("+",lhost) == 0) {
		return 1;                    /* asking for trouble, but ok.. */
	}

	/* Try for raw ip address first. */
	if (isdigit (*lhost) && (laddr = inet_addr (lhost)) != INADDR_NONE)
		return negate * (! (raddr ^ laddr));

	/* Better be a hostname. */
#ifdef __UCLIBC_HAS_THREADS__
	buflen = 1024;
	buffer = malloc(buflen);
	save_errno = errno;

	while (gethostbyname_r (lhost, &hostbuf, buffer, buflen, &hp, &herr)
	       != 0) {
	    free(buffer);
	    return (0);
	}
	free(buffer);
	__set_errno (save_errno);
#else
	hp = gethostbyname(lhost);
#endif /* __UCLIBC_HAS_THREADS__ */

	if (hp == NULL)
		return 0;

	/* Spin through ip addresses. */
	for (pp = hp->h_addr_list; *pp; ++pp)
		if (!memcmp (&raddr, *pp, sizeof (u_int32_t)))
			return negate;

	/* No match. */
	return (0);
}

/* Returns 1 on positive match, 0 on no match, -1 on negative match.  */
static int
__icheckuser (const char *luser, const char *ruser)
{

    /*
      luser is user entry from .rhosts/hosts.equiv file
      ruser is user id on remote host
      */

#ifdef HAVE_NETGROUP
    /* [-+]@netgroup */
    if (strncmp ("+@", luser, 2) == 0)
	return innetgr (&luser[2], NULL, ruser, NULL);

    if (strncmp ("-@", luser,2) == 0)
	return -innetgr (&luser[2], NULL, ruser, NULL);
#endif /* HAVE_NETGROUP */

    /* -user */
    if (strncmp ("-", luser, 1) == 0)
	return -(strcmp (&luser[1], ruser) == 0);

    /* + */
    if (strcmp ("+", luser) == 0)
	return 1;

    /* simple string match */
    return strcmp (ruser, luser) == 0;
}

/*
 * Returns 1 for blank lines (or only comment lines) and 0 otherwise
 */
static int
__isempty(char *p)
{
    while (*p && isspace (*p)) {
	++p;
    }

    return (*p == '\0' || *p == '#') ? 1 : 0 ;
}

/*
 * Returns 0 if positive match, -1 if _not_ ok.
 */
static int
__ivaliduser2(hostf, raddr, luser, ruser, rhost)
	FILE *hostf;
	u_int32_t raddr;
	const char *luser, *ruser, *rhost;
{
    register const char *user;
    register char *p;
    int hcheck, ucheck;
    char *buf = NULL;
    size_t bufsize = 0;
    int retval = -1;

    while (getline (&buf, &bufsize, hostf) > 0) {
	buf[bufsize - 1] = '\0'; /* Make sure it's terminated.  */
        p = buf;

	/* Skip empty or comment lines */
	if (__isempty (p)) {
	    continue;
	}

	/* Skip lines that are too long. */
	if (strchr (p, '\n') == NULL) {
	    int ch = getc_unlocked (hostf);

	    while (ch != '\n' && ch != EOF)
	      ch = getc_unlocked (hostf);
	    continue;
	}

	for (;*p && !isspace(*p); ++p) {
	    *p = tolower (*p);
	}

	/* Next we want to find the permitted name for the remote user.  */
	if (*p == ' ' || *p == '\t') {
	    /* <nul> terminate hostname and skip spaces */
	    for (*p++='\0'; *p && isspace (*p); ++p);

	    user = p;                   /* this is the user's name */
	    while (*p && !isspace (*p))
		++p;                    /* find end of user's name */
	} else
	    user = p;

	*p = '\0';              /* <nul> terminate username (+host?) */

	/* buf -> host(?) ; user -> username(?) */

	/* First check host part */
	hcheck = __icheckhost (raddr, buf, rhost);

	if (hcheck < 0)
	    break;

	if (hcheck) {
	    /* Then check user part */
	    if (! (*user))
		user = luser;

	    ucheck = __icheckuser (user, ruser);

	    /* Positive 'host user' match? */
	    if (ucheck > 0) {
		retval = 0;
		break;
	    }

	    /* Negative 'host -user' match? */
	    if (ucheck < 0)
		break;

	    /* Neither, go on looking for match */
	}
    }

    if (buf != NULL)
      free (buf);

    return retval;
}
