/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* Shared library add-on to ip6tables for condition match */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ip6tables.h>

#include<linux/netfilter_ipv6/ip6_tables.h>
#include<linux/netfilter_ipv6/ip6t_condition.h>


static void
help(void)
{
	printf("condition match v%s options:\n"
	       "--condition [!] filename       "
	       "Match on boolean value stored in /proc file\n",
	       IPTABLES_VERSION);
}


static struct option opts[] = {
	{ .name = "condition", .has_arg = 1, .flag = 0, .val = 'X' },
	{ .name = 0 }
};

static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry, unsigned int *nfcache,
      struct ip6t_entry_match **match)
{
	struct condition6_info *info =
	    (struct condition6_info *) (*match)->data;

	if (c == 'X') {
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple conditions");

		check_inverse(optarg, &invert, &optind, 0);

		if (strlen(argv[optind - 1]) < CONDITION6_NAME_LEN)
			strcpy(info->name, argv[optind - 1]);
		else
			exit_error(PARAMETER_PROBLEM,
				   "File name too long");

		info->invert = invert;
		*flags = 1;
		return 1;
	}

	return 0;
}


static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "Condition match: must specify --condition");
}


static void
print(const struct ip6t_ip6 *ip,
		  const struct ip6t_entry_match *match, int numeric)
{
	const struct condition6_info *info =
	    (const struct condition6_info *) match->data;

	printf("condition %s%s ", (info->invert) ? "!" : "", info->name);
}


static void
save(const struct ip6t_ip6 *ip,
		 const struct ip6t_entry_match *match)
{
	const struct condition6_info *info =
	    (const struct condition6_info *) match->data;

	printf("--condition %s\"%s\" ", (info->invert) ? "! " : "", info->name);
}


static struct ip6tables_match condition = {
	.name = "condition",
	.version = IPTABLES_VERSION,
	.size = IP6T_ALIGN(sizeof(struct condition6_info)),
	.userspacesize = IP6T_ALIGN(sizeof(struct condition6_info)),
	.help = &help,
	.parse = &parse,
	.final_check = &final_check,
	.print = &print,
	.save = &save,
	.extra_opts = opts
};


void
_init(void)
{
	register_match6(&condition);
}
