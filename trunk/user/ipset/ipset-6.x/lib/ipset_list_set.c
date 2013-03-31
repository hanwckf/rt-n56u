/* Copyright 2007-2010 Jozsef Kadlecsik (kadlec@blackhole.kfki.hu)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <libipset/data.h>			/* IPSET_OPT_* */
#include <libipset/parse.h>			/* parser functions */
#include <libipset/print.h>			/* printing functions */
#include <libipset/types.h>			/* prototypes */

/* Parse commandline arguments */
static const struct ipset_arg list_set_create_args[] = {
	{ .name = { "size", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_SIZE,
	  .parse = ipset_parse_uint32,		.print = ipset_print_number,
	},
	{ .name = { "timeout", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_TIMEOUT,
	  .parse = ipset_parse_timeout,		.print = ipset_print_number,
	},
	{ },
};

static const struct ipset_arg list_set_adt_args[] = {
	{ .name = { "timeout", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_TIMEOUT,
	  .parse = ipset_parse_timeout,		.print = ipset_print_number,
	},
	{ .name = { "before", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_NAMEREF,
	  .parse = ipset_parse_before,
	},
	{ .name = { "after", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_NAMEREF,
	  .parse = ipset_parse_after,
	},
	{ },
};

static const char list_set_usage[] =
"create SETNAME list:set\n"
"               [size VALUE] [timeout VALUE]\n"
"add    SETNAME NAME [before|after NAME] [timeout VALUE]\n"
"del    SETNAME NAME [before|after NAME]\n"
"test   SETNAME NAME [before|after NAME]\n\n"
"where NAME are existing set names.\n";

static struct ipset_type ipset_list_set0 = {
	.name = "list:set",
	.alias = { "setlist", NULL },
	.revision = 0,
	.family = NFPROTO_UNSPEC,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_setname,
			.print = ipset_print_name,
			.opt = IPSET_OPT_NAME
		},
	},
	.compat_parse_elem = ipset_parse_name_compat,
	.args = {
		[IPSET_CREATE] = list_set_create_args,
		[IPSET_ADD] = list_set_adt_args,
		[IPSET_DEL] = list_set_adt_args,
		[IPSET_TEST] = list_set_adt_args,
	},
	.mandatory = {
		[IPSET_CREATE] = 0,
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_NAME),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_NAME),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_NAME),
	},
	.full = {
		[IPSET_CREATE] = IPSET_FLAG(IPSET_OPT_SIZE)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_NAME)
			| IPSET_FLAG(IPSET_OPT_BEFORE)
			| IPSET_FLAG(IPSET_OPT_NAMEREF)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_NAME)
			| IPSET_FLAG(IPSET_OPT_BEFORE)
			| IPSET_FLAG(IPSET_OPT_NAMEREF),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_NAME)
			| IPSET_FLAG(IPSET_OPT_BEFORE)
			| IPSET_FLAG(IPSET_OPT_NAMEREF),
	},

	.usage = list_set_usage,
	.description = "Initial revision",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_list_set0);
}
