/*
 * pwcat.c
 *
 * Generate a printable version of the password database
 */
/*
 * Arnold Robbins, arnold@gnu.org, May 1993
 * Public Domain
 */

#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    struct passwd *p;

    while ((p = getpwent()) != NULL)
	printf("%s:%s:%ld:%ld:%s:%s:%s\n",
		p->pw_name, p->pw_passwd, (long) p->pw_uid,
		(long) p->pw_gid, p->pw_gecos, p->pw_dir, p->pw_shell);

    endpwent();
    return 0;
}
