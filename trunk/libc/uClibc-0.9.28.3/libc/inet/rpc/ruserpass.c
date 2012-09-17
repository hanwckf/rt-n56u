/*
 * Copyright (c) 1985, 1993, 1994
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
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _(X)  (X)
/* #include "ftp_var.h" */

static	int token (void);
static	FILE *cfile;

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MACHINE	11

static char tokval[100];

static const char tokstr[] =
{
#define TOK_DEFAULT_IDX	0
  "default\0"
#define TOK_LOGIN_IDX	(TOK_DEFAULT_IDX + sizeof "default")
  "login\0"
#define TOK_PASSWORD_IDX (TOK_LOGIN_IDX + sizeof "login")
  "password\0"
#define TOK_PASSWD_IDX	(TOK_PASSWORD_IDX + sizeof "password")
  "passwd\0"
#define TOK_ACCOUNT_IDX	(TOK_PASSWD_IDX + sizeof "passwd")
  "account\0"
#define TOK_MACHINE_IDX	(TOK_ACCOUNT_IDX + sizeof "account")
  "machine\0"
#define TOK_MACDEF_IDX	(TOK_MACHINE_IDX + sizeof "machine")
  "macdef"
};

static const struct toktab {
	int tokstr_off;
	int tval;
} toktab[]= {
	{ TOK_DEFAULT_IDX,	DEFAULT },
	{ TOK_LOGIN_IDX,	LOGIN },
	{ TOK_PASSWORD_IDX,	PASSWD },
	{ TOK_PASSWD_IDX,	PASSWD },
	{ TOK_ACCOUNT_IDX,	ACCOUNT },
	{ TOK_MACHINE_IDX,	MACHINE },
	{ TOK_MACDEF_IDX,	MACDEF }
};



int ruserpass(const char *host, const char **aname, const char **apass)
{
	char *hdir, *buf, *tmp;
	char myname[1024], *mydomain;
	int t, usedefault = 0;
	struct stat stb;

	/* Give up when running a setuid or setgid app. */
	if ((getuid() != geteuid()) || getgid() != getegid())
	    return -1;
	hdir = getenv("HOME");
	if (hdir == NULL) {
		/* If we can't get HOME, fail instead of trying ".",
		   which is no improvement. */
	  	return -1;
	}

	buf = alloca (strlen(hdir) + 8);
	strcpy(buf, hdir);
	strcat(buf, "/.netrc");
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			printf("%s", buf);
		return (0);
	}
	/* No threads use this stream.  */
#ifdef __UCLIBC_HAS_THREADS__
	__fsetlocking (cfile, FSETLOCKING_BYCALLER);
#endif
	if (gethostname(myname, sizeof(myname)) < 0)
		myname[0] = '\0';
	mydomain = strchr(myname, '.');
	if (mydomain==NULL) {
	    mydomain=myname + strlen(myname);
	}
next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		usedefault = 1;
		/* FALL THROUGH */

	case MACHINE:
		if (!usedefault) {
			if (token() != ID)
				continue;
			/*
			 * Allow match either for user's input host name
			 * or official hostname.  Also allow match of
			 * incompletely-specified host in local domain.
			 */
			if (strcasecmp(host, tokval) == 0)
				goto match;
/*			if (__strcasecmp(hostname, tokval) == 0)
				goto match;
			if ((tmp = strchr(hostname, '.')) != NULL &&
			    __strcasecmp(tmp, mydomain) == 0 &&
			    __strncasecmp(hostname, tokval, tmp-hostname) == 0 &&
			    tokval[tmp - hostname] == '\0')
				goto match; */
			if ((tmp = strchr(host, '.')) != NULL &&
			    strcasecmp(tmp, mydomain) == 0 &&
			    strncasecmp(host, tokval, tmp - host) == 0 &&
			    tokval[tmp - host] == '\0')
				goto match;
			continue;
		}
	match:
		while ((t = token()) && t != MACHINE && t != DEFAULT) switch(t) {

		case LOGIN:
			if (token()) {
				if (*aname == 0) {
				  char *newp;
				  newp = malloc((unsigned) strlen(tokval) + 1);
				  if (newp == NULL)
				    {
				      printf(_("out of memory"));
				      goto bad;
				    }
				  *aname = strcpy(newp, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			}
			break;
		case PASSWD:
			if (strcmp(*aname, "anonymous") &&
			    fstat(fileno(cfile), &stb) >= 0 &&
			    (stb.st_mode & 077) != 0) {
	printf(_("Error: .netrc file is readable by others."));
	printf(_("Remove password or make file unreadable by others."));
				goto bad;
			}
			if (token() && *apass == 0) {
				char *newp;
				newp = malloc((unsigned) strlen(tokval) + 1);
				if (newp == NULL)
				  {
				    printf(_("out of memory"));
				    goto bad;
				  }
				*apass = strcpy(newp, tokval);
			}
			break;
		case ACCOUNT:
#if 0
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
	printf("Error: .netrc file is readable by others.");
	printf("Remove account or make file unreadable by others.");
				goto bad;
			}
			if (token() && *aacct == 0) {
				*aacct = malloc((unsigned) strlen(tokval) + 1);
				(void) strcpy(*aacct, tokval);
			}
#endif
			break;
		case MACDEF:
#if 0
			if (proxy) {
				(void) fclose(cfile);
				return (0);
			}
			while ((c=getc_unlocked(cfile)) != EOF && c == ' '
			       || c == '\t');
			if (c == EOF || c == '\n') {
				printf("Missing macdef name argument.\n");
				goto bad;
			}
			if (macnum == 16) {
				printf("Limit of 16 macros have already been defined\n");
				goto bad;
			}
			tmp = macros[macnum].mac_name;
			*tmp++ = c;
			for (i=0; i < 8 && (c=getc_unlocked(cfile)) != EOF &&
			    !isspace(c); ++i) {
				*tmp++ = c;
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			*tmp = '\0';
			if (c != '\n') {
				while ((c=getc_unlocked(cfile)) != EOF
				       && c != '\n');
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			if (macnum == 0) {
				macros[macnum].mac_start = macbuf;
			}
			else {
				macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
			}
			tmp = macros[macnum].mac_start;
			while (tmp != macbuf + 4096) {
				if ((c=getc_unlocked(cfile)) == EOF) {
				printf("Macro definition missing null line terminator.\n");
					goto bad;
				}
				*tmp = c;
				if (*tmp == '\n') {
					if (*(tmp-1) == '\0') {
					   macros[macnum++].mac_end = tmp - 1;
					   break;
					}
					*tmp = '\0';
				}
				tmp++;
			}
			if (tmp == macbuf + 4096) {
				printf("4K macro buffer exceeded\n");
				goto bad;
			}
#endif
			break;
		default:
			printf(_("Unknown .netrc keyword %s"), tokval);
			break;
		}
		goto done;
	}
done:
	(void) fclose(cfile);
	return (0);
bad:
	(void) fclose(cfile);
	return (-1);
}

static int
token()
{
	char *cp;
	int c;
	int i;

	if (feof_unlocked(cfile) || ferror_unlocked(cfile))
		return (0);
	while ((c = getc_unlocked(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc_unlocked(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc_unlocked(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc_unlocked(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc_unlocked(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (i = 0; i < (int) (sizeof (toktab) / sizeof (toktab[0])); ++i)
		if (!strcmp(&tokstr[toktab[i].tokstr_off], tokval))
			return toktab[i].tval;
	return (ID);
}
