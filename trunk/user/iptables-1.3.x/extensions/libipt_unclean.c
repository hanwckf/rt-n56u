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
/* Shared library add-on to iptables for unclean. */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"unclean v%s takes no options\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{0}
};

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	return 0;
}

/* Final check; must have specified --mac. */
static void final_check(unsigned int flags)
{
}

static
struct iptables_match unclean = { 
	.next		= NULL,
	.name		= "unclean",
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(0),
	.userspacesize	= IPT_ALIGN(0),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= NULL,
	.save		= NULL,
	.extra_opts	= opts
};

void _init(void)
{
	register_match(&unclean);
}
