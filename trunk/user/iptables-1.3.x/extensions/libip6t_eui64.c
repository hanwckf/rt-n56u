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
/* Shared library add-on to ip6tables to add EUI64 address checking support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <ip6tables.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"eui64 v%s options:\n"
" This module hasn't got any option\n"
" This module checks for EUI64 IPv6 addresses\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{0}
};

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      unsigned int *nfcache,
      struct ip6t_entry_match **match)
{
	return 0;
}

/* Final check */
static void final_check(unsigned int flags)
{
}

/* Prints out the matchinfo. */
static void
print(const struct ip6t_ip6 *ip,
      const struct ip6t_entry_match *match,
      int numeric)
{
	printf("eui64 ");
}

/* Saves the union ip6t_matchinfo in parsable form to stdout. */
static void save(const struct ip6t_ip6 *ip, const struct ip6t_entry_match *match)
{

}

static struct ip6tables_match eui64 = {
	.name 		= "eui64",
	.version	= IPTABLES_VERSION,
	.size		= IP6T_ALIGN(sizeof(int)),
	.userspacesize	= IP6T_ALIGN(sizeof(int)),
	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts,
};

void _init(void)
{
	register_match6(&eui64);
}
