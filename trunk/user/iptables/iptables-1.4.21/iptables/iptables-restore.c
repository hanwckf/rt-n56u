/* Code to restore the iptables state, from file by iptables-save.
 * (C) 2000-2002 by Harald Welte <laforge@gnumonks.org>
 * based on previous code from Rusty Russell <rusty@linuxcare.com.au>
 *
 * This code is distributed under the terms of GNU GPL v2
 */

#include <getopt.h>
#include <sys/errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "iptables.h"
#include "xshared.h"
#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

#ifdef DEBUG
#define DEBUGP(x, args...) fprintf(stderr, x, ## args)
#else
#define DEBUGP(x, args...)
#endif

static int counters = 0, verbose = 0, noflush = 0, wait = 0;

static struct timeval wait_interval = {
	.tv_sec	= 1,
};

/* Keeping track of external matches and targets.  */
static const struct option options[] = {
	{.name = "counters",      .has_arg = 0, .val = 'c'},
	{.name = "verbose",       .has_arg = 0, .val = 'v'},
	{.name = "test",          .has_arg = 0, .val = 't'},
	{.name = "help",          .has_arg = 0, .val = 'h'},
	{.name = "noflush",       .has_arg = 0, .val = 'n'},
	{.name = "modprobe",      .has_arg = 1, .val = 'M'},
	{.name = "table",         .has_arg = 1, .val = 'T'},
	{.name = "wait",          .has_arg = 2, .val = 'w'},
	{.name = "wait-interval", .has_arg = 2, .val = 'W'},
	{NULL},
};

static void print_usage(const char *name, const char *version) __attribute__((noreturn));

#define prog_name iptables_globals.program_name

static void print_usage(const char *name, const char *version)
{
	fprintf(stderr, "Usage: %s [-c] [-v] [-t] [-h] [-W usecs]\n"
			"	   [ --binary ]\n"
			"	   [ --counters ]\n"
			"	   [ --verbose ]\n"
			"	   [ --test ]\n"
			"	   [ --help ]\n"
			"	   [ --noflush ]\n"
			"	   [ --wait=<seconds>\n"
			"	   [ --wait-interval=<usecs>\n"
			"	   [ --table=<TABLE> ]\n"
			"	   [ --modprobe=<command>]\n", name);

	exit(1);
}

static struct xtc_handle *create_handle(const char *tablename)
{
	struct xtc_handle *handle;

	handle = iptc_init(tablename);

	if (!handle) {
		/* try to insmod the module if iptc_init failed */
		xtables_load_ko(xtables_modprobe_program, false);
		handle = iptc_init(tablename);
	}

	if (!handle) {
		xtables_error(PARAMETER_PROBLEM, "%s: unable to initialize "
			"table '%s'\n", prog_name, tablename);
		exit(1);
	}
	return handle;
}

static int parse_counters(char *string, struct xt_counters *ctr)
{
	unsigned long long pcnt, bcnt;
	int ret;

	ret = sscanf(string, "[%llu:%llu]", &pcnt, &bcnt);
	ctr->pcnt = pcnt;
	ctr->bcnt = bcnt;
	return ret == 2;
}

/* global new argv and argc */
static char *newargv[255];
static int newargc;

/* function adding one argument to newargv, updating newargc 
 * returns true if argument added, false otherwise */
static int add_argv(char *what) {
	DEBUGP("add_argv: %s\n", what);
	if (what && newargc + 1 < ARRAY_SIZE(newargv)) {
		newargv[newargc] = strdup(what);
		newargv[++newargc] = NULL;
		return 1;
	} else {
		xtables_error(PARAMETER_PROBLEM,
			"Parser cannot handle more arguments\n");
		return 0;
	}
}

static void free_argv(void) {
	int i;

	for (i = 0; i < newargc; i++)
		free(newargv[i]);
}

static void add_param_to_argv(char *parsestart)
{
	int quote_open = 0, escaped = 0, param_len = 0;
	char param_buffer[1024], *curchar;

	/* After fighting with strtok enough, here's now
	 * a 'real' parser. According to Rusty I'm now no
	 * longer a real hacker, but I can live with that */

	for (curchar = parsestart; *curchar; curchar++) {
		if (quote_open) {
			if (escaped) {
				param_buffer[param_len++] = *curchar;
				escaped = 0;
				continue;
			} else if (*curchar == '\\') {
				escaped = 1;
				continue;
			} else if (*curchar == '"') {
				quote_open = 0;
				*curchar = ' ';
			} else {
				param_buffer[param_len++] = *curchar;
				continue;
			}
		} else {
			if (*curchar == '"') {
				quote_open = 1;
				continue;
			}
		}

		if (*curchar == ' '
		    || *curchar == '\t'
		    || * curchar == '\n') {
			if (!param_len) {
				/* two spaces? */
				continue;
			}

			param_buffer[param_len] = '\0';

			/* check if table name specified */
			if (!strncmp(param_buffer, "-t", 2)
			    || !strncmp(param_buffer, "--table", 8)) {
				xtables_error(PARAMETER_PROBLEM,
				"The -t option (seen in line %u) cannot be "
				"used in iptables-restore.\n", line);
				exit(1);
			}

			add_argv(param_buffer);
			param_len = 0;
		} else {
			/* regular character, copy to buffer */
			param_buffer[param_len++] = *curchar;

			if (param_len >= sizeof(param_buffer))
				xtables_error(PARAMETER_PROBLEM,
				   "Parameter too long!");
		}
	}
}

int
iptables_restore_main(int argc, char *argv[])
{
	struct xtc_handle *handle = NULL;
	char buffer[10240];
	int c, lock;
	char curtable[XT_TABLE_MAXNAMELEN + 1];
	FILE *in;
	int in_table = 0, testing = 0;
	const char *tablename = NULL;
	const struct xtc_ops *ops = &iptc_ops;

	line = 0;
	lock = XT_LOCK_NOT_ACQUIRED;

	iptables_globals.program_name = "iptables-restore";
	c = xtables_init_all(&iptables_globals, NFPROTO_IPV4);
	if (c < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
				iptables_globals.program_name,
				iptables_globals.program_version);
		exit(1);
	}
#if defined(ALL_INCLUSIVE) || defined(NO_SHARED_LIBS)
	init_extensions();
	init_extensions4();
#endif

	while ((c = getopt_long(argc, argv, "bcvthnwWM:T:", options, NULL)) != -1) {
		switch (c) {
			case 'b':
				fprintf(stderr, "-b/--binary option is not implemented\n");
				break;
			case 'c':
				counters = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 't':
				testing = 1;
				break;
			case 'h':
				print_usage("iptables-restore",
					    IPTABLES_VERSION);
				break;
			case 'n':
				noflush = 1;
				break;
			case 'w':
				wait = parse_wait_time(argc, argv);
				break;
			case 'W':
				parse_wait_interval(argc, argv, &wait_interval);
				break;
			case 'M':
				xtables_modprobe_program = optarg;
				break;
			case 'T':
				tablename = optarg;
				break;
		}
	}

	if (optind == argc - 1) {
		in = fopen(argv[optind], "re");
		if (!in) {
			fprintf(stderr, "Can't open %s: %s\n", argv[optind],
				strerror(errno));
			exit(1);
		}
	}
	else if (optind < argc) {
		fprintf(stderr, "Unknown arguments found on commandline\n");
		exit(1);
	}
	else in = stdin;

	/* Grab standard input. */
	while (fgets(buffer, sizeof(buffer), in)) {
		int ret = 0;

		line++;
		if (buffer[0] == '\n')
			continue;
		else if (buffer[0] == '#') {
			if (verbose)
				fputs(buffer, stdout);
			continue;
		} else if ((strcmp(buffer, "COMMIT\n") == 0) && (in_table)) {
			if (!testing) {
				DEBUGP("Calling commit\n");
				ret = ops->commit(handle);
				ops->free(handle);
				handle = NULL;
			} else {
				DEBUGP("Not calling commit, testing\n");
				ret = 1;
			}

			/* Done with the current table, release the lock. */
			if (lock >= 0) {
				xtables_unlock(lock);
				lock = XT_LOCK_NOT_ACQUIRED;
			}

			in_table = 0;
		} else if ((buffer[0] == '*') && (!in_table)) {
			/* Acquire a lock before we create a new table handle */
			lock = xtables_lock(wait, &wait_interval);
			if (lock == XT_LOCK_BUSY) {
				fprintf(stderr, "Another app is currently holding the xtables lock. "
					"Perhaps you want to use the -w option?\n");
				exit(RESOURCE_PROBLEM);
			}

			/* New table */
			char *table;

			table = strtok(buffer+1, " \t\n");
			DEBUGP("line %u, table '%s'\n", line, table);
			if (!table) {
				xtables_error(PARAMETER_PROBLEM,
					"%s: line %u table name invalid\n",
					xt_params->program_name, line);
				exit(1);
			}
			strncpy(curtable, table, XT_TABLE_MAXNAMELEN);
			curtable[XT_TABLE_MAXNAMELEN] = '\0';

			if (tablename && (strcmp(tablename, table) != 0))
				continue;
			if (handle)
				ops->free(handle);

			handle = create_handle(table);
			if (noflush == 0) {
				DEBUGP("Cleaning all chains of table '%s'\n",
					table);
				for_each_chain4(flush_entries4, verbose, 1,
						handle);

				DEBUGP("Deleting all user-defined chains "
				       "of table '%s'\n", table);
				for_each_chain4(delete_chain4, verbose, 0,
						handle);
			}

			ret = 1;
			in_table = 1;

		} else if ((buffer[0] == ':') && (in_table)) {
			/* New chain. */
			char *policy, *chain;

			chain = strtok(buffer+1, " \t\n");
			DEBUGP("line %u, chain '%s'\n", line, chain);
			if (!chain) {
				xtables_error(PARAMETER_PROBLEM,
					   "%s: line %u chain name invalid\n",
					   xt_params->program_name, line);
				exit(1);
			}

			if (strlen(chain) >= XT_EXTENSION_MAXNAMELEN)
				xtables_error(PARAMETER_PROBLEM,
					   "Invalid chain name `%s' "
					   "(%u chars max)",
					   chain, XT_EXTENSION_MAXNAMELEN - 1);

			if (ops->builtin(chain, handle) <= 0) {
				if (noflush && ops->is_chain(chain, handle)) {
					DEBUGP("Flushing existing user defined chain '%s'\n", chain);
					if (!ops->flush_entries(chain, handle))
						xtables_error(PARAMETER_PROBLEM,
							   "error flushing chain "
							   "'%s':%s\n", chain,
							   strerror(errno));
				} else {
					DEBUGP("Creating new chain '%s'\n", chain);
					if (!ops->create_chain(chain, handle))
						xtables_error(PARAMETER_PROBLEM,
							   "error creating chain "
							   "'%s':%s\n", chain,
							   strerror(errno));
				}
			}

			policy = strtok(NULL, " \t\n");
			DEBUGP("line %u, policy '%s'\n", line, policy);
			if (!policy) {
				xtables_error(PARAMETER_PROBLEM,
					   "%s: line %u policy invalid\n",
					   xt_params->program_name, line);
				exit(1);
			}

			if (strcmp(policy, "-") != 0) {
				struct xt_counters count;

				if (counters) {
					char *ctrs;
					ctrs = strtok(NULL, " \t\n");

					if (!ctrs || !parse_counters(ctrs, &count))
						xtables_error(PARAMETER_PROBLEM,
							   "invalid policy counters "
							   "for chain '%s'\n", chain);

				} else {
					memset(&count, 0, sizeof(count));
				}

				DEBUGP("Setting policy of chain %s to %s\n",
					chain, policy);

				if (!ops->set_policy(chain, policy, &count,
						     handle))
					xtables_error(OTHER_PROBLEM,
						"Can't set policy `%s'"
						" on `%s' line %u: %s\n",
						policy, chain, line,
						ops->strerror(errno));
			}

			ret = 1;

		} else if (in_table) {
			int a;
			char *ptr = buffer;
			char *pcnt = NULL;
			char *bcnt = NULL;
			char *parsestart;

			/* reset the newargv */
			newargc = 0;

			if (buffer[0] == '[') {
				/* we have counters in our input */
				ptr = strchr(buffer, ']');
				if (!ptr)
					xtables_error(PARAMETER_PROBLEM,
						   "Bad line %u: need ]\n",
						   line);

				pcnt = strtok(buffer+1, ":");
				if (!pcnt)
					xtables_error(PARAMETER_PROBLEM,
						   "Bad line %u: need :\n",
						   line);

				bcnt = strtok(NULL, "]");
				if (!bcnt)
					xtables_error(PARAMETER_PROBLEM,
						   "Bad line %u: need ]\n",
						   line);

				/* start command parsing after counter */
				parsestart = ptr + 1;
			} else {
				/* start command parsing at start of line */
				parsestart = buffer;
			}

			add_argv(argv[0]);
			add_argv("-t");
			add_argv(curtable);

			if (counters && pcnt && bcnt) {
				add_argv("--set-counters");
				add_argv((char *) pcnt);
				add_argv((char *) bcnt);
			}

			add_param_to_argv(parsestart);

			DEBUGP("calling do_command4(%u, argv, &%s, handle):\n",
				newargc, curtable);

			for (a = 0; a < newargc; a++)
				DEBUGP("argv[%u]: %s\n", a, newargv[a]);

			ret = do_command4(newargc, newargv,
					 &newargv[2], &handle, true);

			free_argv();
			fflush(stdout);
		}
		if (tablename && (strcmp(tablename, curtable) != 0))
			continue;
		if (!ret) {
			fprintf(stderr, "%s: line %u failed\n",
					xt_params->program_name, line);
			exit(1);
		}
	}
	if (in_table) {
		fprintf(stderr, "%s: COMMIT expected at line %u\n",
				xt_params->program_name, line + 1);
		exit(1);
	}

	fclose(in);
	return 0;
}
