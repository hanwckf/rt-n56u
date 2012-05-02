/*
 * test_grp.c - This file is part of the libc-8086/grp package for ELKS,
 * Copyright (C) 1995, 1996 Nat Friedman <ndf@linux.mit.edu>.
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <grp.h>

int main(int argc, char **argv)
{
	struct group *group;
	char **tmp_mem;
	int test_gid;

	fprintf(stdout, "Beginning test of libc/grp...\n");

	fprintf(stdout, "=> Testing setgrent(), getgrent(), endgrent()...\n");
	fprintf(stdout, "-> setgrent()...\n");
	setgrent();
	fprintf(stdout, "-> getgrent()...\n");
	printf
		("********************************************************************************\n");
	while ((group = getgrent()) != NULL) {
		printf("gr_name\t\t: %s\n", group->gr_name);
		printf("gr_passwd\t: %s\n", group->gr_passwd);
		printf("gr_gid\t\t: %d\n", (int) group->gr_gid);
		printf("gr_mem\t\t: ");
		fflush(stdout);
		tmp_mem = group->gr_mem;
		while (*tmp_mem != NULL) {
			printf("%s, ", *tmp_mem);
			tmp_mem++;
		}
		printf
			("\n********************************************************************************\n");
	}
	fprintf(stdout, "-> endgrent()...\n");
	endgrent();
	fprintf(stdout,
			"=> Test of setgrent(), getgrent(), endgrent() complete.\n");
	fprintf(stdout, "=> Testing getgrid(), getgrnam()...\n");
	fprintf(stdout, "-> getgrgid()...\n");
	printf
		("********************************************************************************\n");
	for (test_gid = 0; test_gid < 100; test_gid++) {
		fprintf(stdout, "-> getgrgid(%d)...\n", test_gid);
		group = getgrgid((gid_t) test_gid);
		if (group != NULL) {
			printf("gr_name\t: %s\n", group->gr_name);
			printf("gr_passwd\t: %s\n", group->gr_passwd);
			printf("gr_gid\t: %d\n", (int) group->gr_gid);
			printf("gr_mem\t\t: ");
			fflush(stdout);
			tmp_mem = group->gr_mem;
			while (*tmp_mem != NULL) {
				printf("%s, ", *tmp_mem);
				tmp_mem++;
			}
		}
		printf
			("\n********************************************************************************\n");
	}
	fprintf(stdout, "-> getgrnam()...\n");
	group = getgrnam("root");
	if (group == NULL) {
		printf(">NULL<\n");
	} else {
		printf("gr_name\t: %s\n", group->gr_name);
		printf("gr_passwd\t: %s\n", group->gr_passwd);
		printf("gr_gid\t: %d\n", (int) group->gr_gid);
		printf("gr_mem\t\t: ");
		fflush(stdout);
		tmp_mem = group->gr_mem;
		while (*tmp_mem != NULL) {
			printf("%s, ", *tmp_mem);
			tmp_mem++;
		}
		printf("\n");
	}


	return 0;
}
