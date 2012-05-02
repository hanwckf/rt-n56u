/*
 * test_pwd.c - This file is part of the libc-8086/pwd package for ELKS,
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
#include <pwd.h>

int main(int argc, char **argv)
{
	struct passwd *passwd;
	int test_uid;

	fprintf(stdout, "Beginning test of libc/pwd...\n");

	fprintf(stdout, "=> Testing setpwent(), getpwent(), endpwent()...\n");
	fprintf(stdout, "-> setpwent()...\n");
	setpwent();
	fprintf(stdout, "-> getpwent()...\n");
	printf
		("********************************************************************************\n");
	while ((passwd = getpwent()) != NULL) {
		printf("pw_name\t\t: %s\n", passwd->pw_name);
		printf("pw_passwd\t: %s\n", passwd->pw_passwd);
		printf("pw_uid\t\t: %d\n", (int) passwd->pw_uid);
		printf("pw_gid\t\t: %d\n", (int) passwd->pw_gid);
		printf("pw_gecos\t: %s\n", passwd->pw_gecos);
		printf("pw_dir\t\t: %s\n", passwd->pw_dir);
		printf("pw_shell\t: %s\n", passwd->pw_shell);
		printf
			("********************************************************************************\n");
	}
	fprintf(stdout, "-> endpwent()...\n");
	endpwent();
	fprintf(stdout,
			"=> Test of setpwent(), getpwent(), endpwent() complete.\n");
	fprintf(stdout, "=> Testing getpwuid(), getpwnam()...\n");
	fprintf(stdout, "-> getpwuid()...\n");
	printf
		("********************************************************************************\n");
	for (test_uid = 0; test_uid < 1000; test_uid++) {
		fprintf(stdout, "-> getpwuid(%d)...\n", test_uid);
		passwd = getpwuid((uid_t) test_uid);
		if (passwd != NULL) {
			printf("pw_name\t\t: %s\n", passwd->pw_name);
			printf("pw_passwd\t: %s\n", passwd->pw_passwd);
			printf("pw_uid\t\t: %d\n", (int) passwd->pw_uid);
			printf("pw_gid\t\t: %d\n", (int) passwd->pw_gid);
			printf("pw_gecos\t: %s\n", passwd->pw_gecos);
			printf("pw_dir\t\t: %s\n", passwd->pw_dir);
			printf("pw_shell\t: %s\n", passwd->pw_shell);
			printf
				("********************************************************************************\n");
		}
	}
	fprintf(stdout, "-> getpwnam()...\n");
	passwd = getpwnam("root");
	if (passwd == NULL) {
		printf(">NULL<\n");
	} else {
		printf("pw_name\t\t: %s\n", passwd->pw_name);
		printf("pw_passwd\t: %s\n", passwd->pw_passwd);
		printf("pw_uid\t\t: %d\n", (int) passwd->pw_uid);
		printf("pw_gid\t\t: %d\n", (int) passwd->pw_gid);
		printf("pw_gecos\t: %s\n", passwd->pw_gecos);
		printf("pw_dir\t\t: %s\n", passwd->pw_dir);
		printf("pw_shell\t: %s\n", passwd->pw_shell);
	}
	return 0;
}
