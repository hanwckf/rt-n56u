/* vi: set sw=4 ts=4: */
/*
 * Some simple macros for use in test applications.
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef TESTSUITE_H
#define TESTSUITE_H

#ifdef __NO_TESTCODE__

extern size_t test_number;

extern void init_testsuite(const char* testname);
extern void done_testing(void) __attribute__((noreturn));
extern void success_msg(int result, const char* command);
extern void error_msg(int result, int line, const char* file, const char* command);

#else

size_t test_number = 0;
static int failures = 0;

void error_msg(int result, int line, const char* file, const char* command)
{
	failures++;

	printf("\nFAILED TEST %lu: \n\t%s\n", (unsigned long)test_number, command);
	printf("AT LINE: %d, FILE: %s\n\n", line, file);
}   

void success_msg(int result, const char* command)
{
#if 0
	printf("passed test: %s == 0\n", command);
#endif	
}

void done_testing(void)
{
    if (0 < failures) {
		printf("Failed %d tests\n", failures);
		exit(EXIT_FAILURE);
	} else {
		printf("All functions tested sucessfully\n");
		exit(EXIT_SUCCESS);
	}
}

void init_testsuite(const char* testname)
{
	printf("%s", testname);
	test_number = 0;
	failures = 0;
	atexit(done_testing);
}

#endif /* __NO_TESTCODE__ */


#define TEST_STRING_OUTPUT(command, expected_result) \
	do { \
		int result = strcmp(command, expected_result); \
		test_number++; \
		if (result == expected_result) { \
			success_msg(result, "command"); \
		} else { \
			error_msg(result, __LINE__, __FILE__, command); \
		}; \
	} while (0)

#define TEST_NUMERIC(command, expected_result) \
	do { \
		int result = (command); \
		test_number++; \
		if (result == expected_result) { \
			success_msg(result, # command); \
		} else { \
			error_msg(result, __LINE__, __FILE__, # command); \
		}; \
	} while (0)

#define TEST(command) \
	do { \
		int result = (command); \
		test_number++; \
		if (result == 1) { \
			success_msg(result, # command); \
		} else { \
			error_msg(result, __LINE__, __FILE__,  # command); \
		}; \
	} while (0)

#define STR_CMD(cmd) cmd
		
#endif	/* TESTSUITE_H */
