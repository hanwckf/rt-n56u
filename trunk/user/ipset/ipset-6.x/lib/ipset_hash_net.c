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
static const struct ipset_arg hash_net_create_args[] = {
	{ .name = { "family", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_FAMILY,
	  .parse = ipset_parse_family,		.print = ipset_print_family,
	},
	/* Alias: family inet */
	{ .name = { "-4", NULL },
	  .has_arg = IPSET_NO_ARG,		.opt = IPSET_OPT_FAMILY,
	  .parse = ipset_parse_family,
	},
	/* Alias: family inet6 */
	{ .name = { "-6", NULL },
	  .has_arg = IPSET_NO_ARG,		.opt = IPSET_OPT_FAMILY,
	  .parse = ipset_parse_family,
	},
	{ .name = { "hashsize", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_HASHSIZE,
	  .parse = ipset_parse_uint32,		.print = ipset_print_number,
	},
	{ .name = { "maxelem", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_MAXELEM,
	  .parse = ipset_parse_uint32,		.print = ipset_print_number,
	},
	{ .name = { "timeout", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_TIMEOUT,
	  .parse = ipset_parse_timeout,		.print = ipset_print_number,
	},
	/* Ignored options: backward compatibilty */
	{ .name = { "probes", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_PROBES,
	  .parse = ipset_parse_ignored,		.print = ipset_print_number,
	},
	{ .name = { "resize", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_RESIZE,
	  .parse = ipset_parse_ignored,		.print = ipset_print_number,
	},
	{ },
};

static const struct ipset_arg hash_net_add_args[] = {
	{ .name = { "timeout", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_TIMEOUT,
	  .parse = ipset_parse_timeout,		.print = ipset_print_number,
	},
	{ },
};

static const char hash_net0_usage[] =
"create SETNAME hash:net\n"
"		[family inet|inet6]\n"
"               [hashsize VALUE] [maxelem VALUE]\n"
"               [timeout VALUE]\n"
"add    SETNAME IP[/CIDR] [timeout VALUE]\n"
"del    SETNAME IP[/CIDR]\n"
"test   SETNAME IP[/CIDR]\n\n"
"where depending on the INET family\n"
"      IP is an IPv4 or IPv6 address (or hostname),\n"
"      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n";

static struct ipset_type ipset_hash_net0 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 0,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ipnet,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.args = {
		[IPSET_CREATE] = hash_net_create_args,
		[IPSET_ADD] = hash_net_add_args,
	},
	.mandatory = {
		[IPSET_CREATE] = 0,
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP),
	},
	.full = {
		[IPSET_CREATE] = IPSET_FLAG(IPSET_OPT_HASHSIZE)
			| IPSET_FLAG(IPSET_OPT_MAXELEM)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR),
	},

	.usage = hash_net0_usage,
	.description = "Initial revision",
};

static const char hash_net1_usage[] =
"create SETNAME hash:net\n"
"		[family inet|inet6]\n"
"               [hashsize VALUE] [maxelem VALUE]\n"
"               [timeout VALUE]\n"
"add    SETNAME IP[/CIDR]|FROM-TO [timeout VALUE]\n"
"del    SETNAME IP[/CIDR]|FROM-TO\n"
"test   SETNAME IP[/CIDR]\n\n"
"where depending on the INET family\n"
"      IP is an IPv4 or IPv6 address (or hostname),\n"
"      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
"      IP range is not supported with IPv6.\n";

static struct ipset_type ipset_hash_net1 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 1,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.args = {
		[IPSET_CREATE] = hash_net_create_args,
		[IPSET_ADD] = hash_net_add_args,
	},
	.mandatory = {
		[IPSET_CREATE] = 0,
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP),
	},
	.full = {
		[IPSET_CREATE] = IPSET_FLAG(IPSET_OPT_HASHSIZE)
			| IPSET_FLAG(IPSET_OPT_MAXELEM)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR)
			| IPSET_FLAG(IPSET_OPT_IP_TO)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR)
			| IPSET_FLAG(IPSET_OPT_IP_TO),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR),
	},

	.usage = hash_net1_usage,
	.description = "Add/del range support",
};

static const struct ipset_arg hash_net2_add_args[] = {
	{ .name = { "timeout", NULL },
	  .has_arg = IPSET_MANDATORY_ARG,	.opt = IPSET_OPT_TIMEOUT,
	  .parse = ipset_parse_timeout,		.print = ipset_print_number,
	},
	{ .name = { "nomatch", NULL },
	  .has_arg = IPSET_NO_ARG,		.opt = IPSET_OPT_NOMATCH,
	  .parse = ipset_parse_flag,		.print = ipset_print_flag,
	},
	{ },
};

static const char hash_net2_usage[] =
"create SETNAME hash:net\n"
"		[family inet|inet6]\n"
"               [hashsize VALUE] [maxelem VALUE]\n"
"               [timeout VALUE]\n"
"add    SETNAME IP[/CIDR]|FROM-TO [timeout VALUE] [nomatch]\n"
"del    SETNAME IP[/CIDR]|FROM-TO\n"
"test   SETNAME IP[/CIDR]\n\n"
"where depending on the INET family\n"
"      IP is an IPv4 or IPv6 address (or hostname),\n"
"      CIDR is a valid IPv4 or IPv6 CIDR prefix.\n"
"      IP range is not supported with IPv6.\n";

static struct ipset_type ipset_hash_net2 = {
	.name = "hash:net",
	.alias = { "nethash", NULL },
	.revision = 2,
	.family = NFPROTO_IPSET_IPV46,
	.dimension = IPSET_DIM_ONE,
	.elem = {
		[IPSET_DIM_ONE - 1] = {
			.parse = ipset_parse_ip4_net6,
			.print = ipset_print_ip,
			.opt = IPSET_OPT_IP
		},
	},
	.args = {
		[IPSET_CREATE] = hash_net_create_args,
		[IPSET_ADD] = hash_net2_add_args,
	},
	.mandatory = {
		[IPSET_CREATE] = 0,
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP),
	},
	.full = {
		[IPSET_CREATE] = IPSET_FLAG(IPSET_OPT_HASHSIZE)
			| IPSET_FLAG(IPSET_OPT_MAXELEM)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT),
		[IPSET_ADD] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR)
			| IPSET_FLAG(IPSET_OPT_IP_TO)
			| IPSET_FLAG(IPSET_OPT_TIMEOUT)
			| IPSET_FLAG(IPSET_OPT_NOMATCH),
		[IPSET_DEL] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR)
			| IPSET_FLAG(IPSET_OPT_IP_TO),
		[IPSET_TEST] = IPSET_FLAG(IPSET_OPT_IP)
			| IPSET_FLAG(IPSET_OPT_CIDR),
	},

	.usage = hash_net2_usage,
	.description = "nomatch flag support",
};

void _init(void);
void _init(void)
{
	ipset_type_add(&ipset_hash_net0);
	ipset_type_add(&ipset_hash_net1);
	ipset_type_add(&ipset_hash_net2);
}
