/* exercise a bug found in malloc-standard when alignment
 * values are out of whack and cause a small overflow into
 * actual user data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define ok(p) ((void*)p > (void*)0x1000)
#define x \
	do { \
		printf("%i: phead = %p, phead->link @ %p = %p %s\n", \
			__LINE__, phead, \
			ok(phead) ? &phead->link : 0, \
			ok(phead) ? phead->link : 0, \
			ok(phead) ? phead->link == 0 ? "" : "!!!!!!!!!!!" : ""); \
		if (phead->link != NULL) exit(1); \
	} while (0);

struct llist_s {
	void *data;
	struct llist_s *link;
} *phead;

int main(int argc, char *argv[])
{
	char *line, *reg;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	phead = malloc(sizeof(*phead));
	phead->link = NULL;

x	line = malloc(80);
x	line = realloc(line, 2);
x	reg = malloc(32);
x	free(line);

x	return 0;
}
