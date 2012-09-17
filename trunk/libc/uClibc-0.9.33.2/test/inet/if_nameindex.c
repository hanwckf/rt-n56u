/* if_nameindex.c: test the if_nameindex() function
 *
 * Copyright (C) 2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

static char ifname[IF_NAMESIZE];

static void test_if_nameindex(void)
{
	size_t i;
	struct if_nameindex *ret;

	ret = if_nameindex();

	if (ret == NULL) {
		perror("if_nameindex()");
		exit(1);
	}

	printf("--- if_nameindex()\n");
	for (i=0; ret[i].if_name; ++i)
		printf("%i: %s\n", ret[i].if_index, ret[i].if_name);

	if_freenameindex(ret);
}

static void test_if_indextoname(void)
{
	if (if_indextoname(1, ifname) == NULL) {
		perror("if_nameindex()");
		exit(1);
	}

	printf("if_indextoname(1) = %s\n", ifname);
}

static void test_if_nametoindex(void)
{
	int ifindex = if_nametoindex(ifname);

	if (ifindex == 0) {
		perror("if_nametoindex()");
		exit(1);
	}

	printf("if_nametoindex(%s) = %i\n", ifname, ifindex);
}

int main(void)
{
	test_if_nameindex();
	test_if_indextoname();
	test_if_nametoindex();
	return 0;
}
