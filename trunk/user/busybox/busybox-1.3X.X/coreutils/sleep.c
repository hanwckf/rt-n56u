/* vi: set sw=4 ts=4: */
/*
 * sleep implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Rewritten to do proper arg and error checking.
 * Also, added a 'fancy' configuration to accept multiple args with
 * time suffixes for seconds, minutes, hours, and days.
 */
//config:config SLEEP
//config:	bool "sleep (2 kb)"
//config:	default y
//config:	help
//config:	sleep is used to pause for a specified number of seconds.
//config:	It comes in 3 versions:
//config:	- small: takes one integer parameter
//config:	- fancy: takes multiple integer arguments with suffixes:
//config:		sleep 1d 2h 3m 15s
//config:	- fancy with fractional numbers:
//config:		sleep 2.3s 4.5h sleeps for 16202.3 seconds
//config:	Last one is "the most compatible" with coreutils sleep,
//config:	but it adds around 1k of code.
//config:
//config:config FEATURE_FANCY_SLEEP
//config:	bool "Enable multiple arguments and s/m/h/d suffixes"
//config:	default y
//config:	depends on SLEEP
//config:	help
//config:	Allow sleep to pause for specified minutes, hours, and days.

/* Do not make this applet NOFORK. It breaks ^C-ing of pauses in shells */
//applet:IF_SLEEP(APPLET(sleep, BB_DIR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_SLEEP) += sleep.o

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU issues -- fancy version matches except args must be ints. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/sleep.html */

//usage:#define sleep_trivial_usage
//usage:	IF_FEATURE_FANCY_SLEEP("[") "N" IF_FEATURE_FANCY_SLEEP("]...")
//usage:#define sleep_full_usage "\n\n"
//usage:	IF_NOT_FEATURE_FANCY_SLEEP("Pause for N seconds")
//usage:	IF_FEATURE_FANCY_SLEEP(
//usage:       "Pause for a time equal to the total of the args given, where each arg can\n"
//usage:       "have an optional suffix of (s)econds, (m)inutes, (h)ours, or (d)ays")
//usage:
//usage:#define sleep_example_usage
//usage:       "$ sleep 2\n"
//usage:       "[2 second delay results]\n"
//usage:	IF_FEATURE_FANCY_SLEEP(
//usage:       "$ sleep 1d 3h 22m 8s\n"
//usage:       "[98528 second delay results]\n")

#include "libbb.h"

int sleep_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int sleep_main(int argc UNUSED_PARAM, char **argv)
{
	duration_t duration;

	++argv;
	if (!*argv)
		bb_show_usage();

	/* GNU sleep accepts "inf", "INF", "infinity" and "INFINITY" */
	if (strncasecmp(argv[0], "inf", 3) == 0)
		for (;;)
			sleep(INT_MAX);

#if ENABLE_FEATURE_FANCY_SLEEP
# if ENABLE_FLOAT_DURATION
	/* undo busybox.c setlocale */
	setlocale(LC_NUMERIC, "C");
# endif
	duration = 0;
	do {
		duration += parse_duration_str(*argv);
	} while (*++argv);
	sleep_for_duration(duration);
#else /* simple */
	duration = xatou(*argv);
	sleep(duration);
#endif
	return EXIT_SUCCESS;
}
