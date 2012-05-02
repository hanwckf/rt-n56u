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
/*
 * Automatic port forwarding target. When this target is entered, a
 * related connection to a port in the reply direction will be
 * expected. This connection may be mapped to a different port.
 *
 * Copyright 2004, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: libipt_autofw.c,v 1.1.1.1 2007/02/15 12:13:46 jiahao Exp $
 */

/* Shared library add-on to iptables to add masquerade support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_autofw.h>
#include <net/netfilter/nf_nat.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"autofw v%s options:\n"
" --related-proto proto\n"
"				Related protocol\n"
" --related-dport port[-port]\n"
"				Related destination port range\n"
" --related-to port[-port]\n"
"				Port range to map related destination port range to.\n\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "related-proto", 1, 0, '1' },
	{ "related-dport", 1, 0, '2' },
	{ "related-to", 1, 0, '3' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

/* Parses ports */
static void
parse_ports(const char *arg, u_int16_t *ports)
{
	const char *dash;
	int port;

	port = atoi(arg);
	if (port == 0 || port > 65535)
		exit_error(PARAMETER_PROBLEM, "Port `%s' not valid\n", arg);

	dash = strchr(arg, '-');
	if (!dash)
		ports[0] = ports[1] = htons(port);
	else {
		int maxport;

		maxport = atoi(dash + 1);
		if (maxport == 0 || maxport > 65535)
			exit_error(PARAMETER_PROBLEM,
				   "Port `%s' not valid\n", dash+1);
		if (maxport < port)
			/* People are stupid. */
			exit_error(PARAMETER_PROBLEM,
				   "Port range `%s' funky\n", arg);
		ports[0] = htons(port);
		ports[1] = htons(maxport);
	}
}


/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	//printf("autofw: parse\n");	// test
	struct ip_autofw_info *info = (struct ip_autofw_info *)(*target)->data;


	switch (c) {
	case '1':
		if (!strcasecmp(optarg, "tcp"))
			info->proto = IPPROTO_TCP;
		else if (!strcasecmp(optarg, "udp"))
			info->proto = IPPROTO_UDP;
		else
			exit_error(PARAMETER_PROBLEM,
				   "unknown protocol `%s' specified", optarg);
		return 1;

	case '2':
		if (check_inverse(optarg, &invert, &optind, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --related-dport");

		parse_ports(optarg, info->dport);
		return 1;

	case '3':
		if (check_inverse(optarg, &invert, &optind, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --related-to");

		parse_ports(optarg, info->to);
		*flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
		return 1;

	default:
		return 0;
	}
}

/* Final check; don't care. */
static void final_check(unsigned int flags)
{
	//printf("autofw: final check[%d]\n", flags);	// tmp test
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	//printf("autofw: print\n");	// tmp test
	struct ip_autofw_info *info = (struct ip_autofw_info *)target->data;

	if (info->proto == IPPROTO_TCP)
		printf("tcp ");
	else if (info->proto == IPPROTO_UDP)
		printf("udp ");
	printf("dpt:%hu", ntohs(info->dport[0]));
	if (ntohs(info->dport[1]) > ntohs(info->dport[0]))
		printf("-%hu", ntohs(info->dport[1]));
	printf(" ");
	printf("to:%hu", ntohs(info->to[0]));
	if (ntohs(info->to[1]) > ntohs(info->to[0]))
		printf("-%hu", ntohs(info->to[1]));
	printf(" ");
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	//printf("autofw: save\n");	// tmp test
	struct ip_autofw_info *info = (struct ip_autofw_info *)target->data;

	printf("--related-proto ");
	if (info->proto == IPPROTO_TCP)
		printf("tcp ");
	else if (info->proto == IPPROTO_UDP)
		printf("udp ");
	printf("--related-dport %hu-%hu ", ntohs(info->dport[0]), ntohs(info->dport[1]));
	printf("--related-to %hu-%hu ", ntohs(info->to[0]), ntohs(info->to[1]));
}

/*
struct iptables_target autofw
= { NULL,
    "autofw",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ip_autofw_info)),
    IPT_ALIGN(sizeof(struct ip_autofw_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};
*/

static struct iptables_target autofw = {	// 1001 edit
        .next           = NULL,
        .name           = "autofw",
        .version        = IPTABLES_VERSION,
        .size           = IPT_ALIGN(sizeof(struct ip_autofw_info)),
        .userspacesize  = IPT_ALIGN(sizeof(struct ip_autofw_info)),
        .help           = &help,
        .parse          = &parse,
        .final_check    = &final_check,
        .print          = &print,
        .save           = &save,
        .extra_opts     = opts
};


void _init(void)
{
	register_target(&autofw);
}
