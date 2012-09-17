/* This test was ripped out of GNU 'id' from coreutils-5.0
 * by Erik Andersen.
 *
 *
 * id is Copyright (C) 1989-2003 Free Software Foundation, Inc.
 * and licensed under the GPL v2 or later, and was written by
 * Arnold Robbins, with a major rewrite by David MacKenzie,
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <err.h>

/* The number of errors encountered so far. */
static int problems = 0;

/* Print the name or value of group ID GID. */
static void print_group(gid_t gid)
{
	struct group *grp = NULL;

	grp = getgrgid(gid);
	if (grp == NULL) {
		warn("cannot find name for group ID %u", gid);
		problems++;
	}

	if (grp == NULL)
		printf("%u", (unsigned)gid);
	else
		printf("%s", grp->gr_name);
}

static int xgetgroups(gid_t gid, int *n_groups, gid_t ** groups)
{
	int max_n_groups;
	int ng;
	gid_t *g;
	int fail = 0;

	max_n_groups = getgroups(0, NULL);

	/* Add 1 just in case max_n_groups is zero.  */
	g = (gid_t *) malloc(max_n_groups * sizeof(gid_t) + 1);
	if (g == NULL)
		err(EXIT_FAILURE, "out of memory");
	ng = getgroups(max_n_groups, g);

	if (ng < 0) {
		warn("cannot get supplemental group list");
		++fail;
		free(g);
	}
	if (!fail) {
		*n_groups = ng;
		*groups = g;
	}
	return fail;
}

/* Print all of the distinct groups the user is in. */
int main(int argc, char *argv[])
{
	struct passwd *pwd;

	pwd = getpwuid(getuid());
	if (pwd == NULL)
		problems++;

	print_group(getgid());
	if (getegid() != getgid()) {
		putchar(' ');
		print_group(getegid());
	}

	{
		int n_groups = 0;
		gid_t *groups;
		register int i;

		if (xgetgroups((pwd ? pwd->pw_gid : (gid_t) - 1),
			       &n_groups, &groups)) {
			return ++problems;
		}

		for (i = 0; i < n_groups; i++)
			if (groups[i] != getgid() && groups[i] != getegid()) {
				putchar(' ');
				print_group(groups[i]);
			}
		free(groups);
	}
	putchar('\n');
	return (problems != 0);
}
