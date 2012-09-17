/*
 * grcat.c
 *
 * Generate a printable version of the group database
 */
/*
 * Arnold Robbins, arnold@gnu.org, May 1993
 * Public Domain
 */

#include <stdlib.h>
#include <stdio.h>
#include <grp.h>

int main(int argc, char **argv)
{
    struct group *g;
    int i;

    while ((g = getgrent()) != NULL) {
	printf("%s:%s:%ld:", g->gr_name, g->gr_passwd,
		(long) g->gr_gid);
	for (i = 0; g->gr_mem[i] != NULL; i++) {
	    printf("%s", g->gr_mem[i]);
	    if (g->gr_mem[i+1] != NULL)
		putchar(',');
	}
	putchar('\n');
    }
    endgrent();
    return 0;
}
