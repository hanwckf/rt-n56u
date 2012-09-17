/*
 * Copyright (c) 1985, 1993
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
 *
 * This version has been hevily modified for use with uClibc
 * November 2002, Erik Andersen <andersen@codepoet.org> 
 */

#define _GNU_SOURCE
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <paths.h>

/*
 * Local shells should NOT be added here.  They should be added in
 * /etc/shells.
 */

static const char * const validsh[] = { _PATH_BSHELL, _PATH_CSHELL, NULL };
static char **curshell, **shells, *strings;
static char **initshells __P((void));

/*
 * Get a list of shells from _PATH_SHELLS, if it exists.
 */
char * getusershell(void)
{
    char *ret;

    if (curshell == NULL)
	curshell = initshells();
    ret = *curshell;
    if (ret != NULL)
	curshell++;
    return (ret);
}

static void __free_initshell_memory(void)
{
    if (shells != NULL) {
	free(shells);
    }
    shells = NULL;
    if (strings != NULL) {
	free(strings);
    }
    strings = NULL;
}

void endusershell(void)
{
    __free_initshell_memory();
    curshell = NULL;
}

void setusershell(void)
{

    curshell = initshells();
}

static char ** initshells(void)
{
    register char **sp, *cp;
    register FILE *fp;
    struct stat statb;
    int flen;

    __free_initshell_memory();

    if ((fp = fopen(_PATH_SHELLS, "r")) == NULL)
	return (char **) validsh;
    if (fstat(fileno(fp), &statb) == -1) {
	goto cleanup;
    }
    if ((strings = malloc((unsigned)statb.st_size + 1)) == NULL) {
	goto cleanup;
    }
    if ((shells = calloc((unsigned)statb.st_size / 3, sizeof (char *))) == NULL) {
	goto cleanup;
    }
    /* No threads using this stream.  */
#ifdef __UCLIBC_HAS_THREADS__
    __fsetlocking (fp, FSETLOCKING_BYCALLER);
#endif
    sp = shells;
    cp = strings;
    flen = statb.st_size;
    while (fgets_unlocked(cp, flen - (cp - strings), fp) != NULL) {
	while (*cp != '#' && *cp != '/' && *cp != '\0')
	    cp++;
	if (*cp == '#' || *cp == '\0')
	    continue;
	*sp++ = cp;
	while (!isspace(*cp) && *cp != '#' && *cp != '\0')
	    cp++;
	*cp++ = '\0';
    }
    *sp = NULL;
    fclose(fp);
    return (shells);

cleanup:
    __free_initshell_memory();
    fclose(fp);
    return (char **) validsh;
}
