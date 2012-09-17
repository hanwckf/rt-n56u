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
/* Shared library add-on to iptables for standard target support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <iptables.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"Standard v%s options:\n"
"(If target is DROP, ACCEPT, RETURN or nothing)\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{0}
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	return 0;
}

/* Final check; don't care. */
static void final_check(unsigned int flags)
{
}

/* Saves the targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
}

static
struct iptables_target standard = { 
	.next		= NULL,
	.name		= "standard",
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(sizeof(int)),
	.userspacesize	= IPT_ALIGN(sizeof(int)),
	.help		= &help,
	.init		= &init,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= NULL,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	register_target(&standard);
}
