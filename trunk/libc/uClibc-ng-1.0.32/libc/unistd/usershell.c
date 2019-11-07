/* setusershell(), getusershell(), endusershell() for uClibc.
 *
 * Copyright (C) 2010 Bernhard Reutner-Fischer <uclibc@uclibc.org>
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in
 * this tarball.
 */
/* My manpage reads:
 * The  getusershell() function returns the next line from the file
 * /etc/shells, opening the file if necessary. The line should contain
 * the pathname of a valid user shell.  If /etc/shells does not exist
 * or is unreadable, getusershell() behaves as if /bin/sh and /bin/csh
 * were listed in the file.
 * The getusershell() function returns a NULL pointer on end-of-file.
 */
#include <unistd.h>
#include <stdlib.h>
#include <paths.h>
#include <string.h>
#include "internal/parse_config.h"

#if defined __USE_BSD || (defined __USE_XOPEN && !defined __USE_UNIX98)

static const char * const defaultsh[] = { _PATH_BSHELL, _PATH_CSHELL, NULL};
static char *shellb, **shells;
static parser_t *shellp;

void endusershell(void)
{
	if (shellp) {
		shells = (char**) shellb;
		while (shells && *shells) {
			char*xxx = *shells++;
			free(xxx);
		}
		config_close(shellp);
		shellp = NULL;
	}
	free(shellb);
	shellb = NULL;
	shells = NULL;
}
libc_hidden_def(endusershell)

void setusershell(void)
{
	endusershell();
	shellp = config_open(_PATH_SHELLS);
	if (shellp == NULL)
		shells = (char **)defaultsh;
	else {
		char **shell = NULL;
		int pos = 0;

		while (config_read(shellp, &shell, 1, 1, "# \t", PARSE_NORMAL))
		{
			shellb = realloc(shellb, (pos + 2) * sizeof(char*));
			shells = (char**) shellb + pos++;
			*shells++ = strdup(*shell);
			*shells = NULL;

		}
		shells = (char **)shellb;
	}
}
libc_hidden_def(setusershell)

char *getusershell(void)
{
	char *sh;
	if (shells == NULL)
		setusershell();
	sh = *shells;
	if (sh)
		shells++;
	return sh;
}
#endif
