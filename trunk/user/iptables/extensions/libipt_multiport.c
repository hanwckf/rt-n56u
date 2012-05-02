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
/* Shared library add-on to iptables to add multiple TCP port support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
/* To ensure that iptables compiles with an old kernel */
#include "../include/linux/netfilter_ipv4/ipt_multiport.h"

/* Function which prints out usage message. */
static void
help_v1(void)
{
	printf(
"multiport v%s options:\n"
" --source-ports [!] port[,port:port,port...]\n"
" --sports ...\n"
"				match source port(s)\n"
" --destination-ports [!] port[,port:port,port...]\n"
" --dports ...\n"
"				match destination port(s)\n"
" --ports [!] port[,port:port,port]\n"
"				match both source and destination port(s)\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "source-ports", 1, 0, '1' },
	{ "sports", 1, 0, '1' }, /* synonym */
	{ "destination-ports", 1, 0, '2' },
	{ "dports", 1, 0, '2' }, /* synonym */
	{ "ports", 1, 0, '3' },
	{0}
};

static char *
proto_to_name(u_int8_t proto)
{
	switch (proto) {
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_UDP:
		return "udp";
	case IPPROTO_UDPLITE:
		return "udplite";
	case IPPROTO_SCTP:
		return "sctp";
	case IPPROTO_DCCP:
		return "dccp";
	default:
		return NULL;
	}
}

static void
parse_multi_ports_v1(const char *portstring, 
		     struct ipt_multiport_v1 *multiinfo,
		     const char *proto)
{
	char *buffer, *cp, *next, *range;
	unsigned int i;
	u_int16_t m;

	buffer = strdup(portstring);
	if (!buffer) exit_error(OTHER_PROBLEM, "strdup failed");

	for (i=0; i<IPT_MULTI_PORTS; i++)
		multiinfo->pflags[i] = 0;
 
	for (cp=buffer, i=0; cp && i<IPT_MULTI_PORTS; cp=next, i++) {
		next=strchr(cp, ',');
 		if (next) *next++='\0';
		range = strchr(cp, ':');
		if (range) {
			if (i == IPT_MULTI_PORTS-1)
				exit_error(PARAMETER_PROBLEM,
					   "too many ports specified");
			*range++ = '\0';
		}
		multiinfo->ports[i] = parse_port(cp, proto);
		if (range) {
			multiinfo->pflags[i] = 1;
			multiinfo->ports[++i] = parse_port(range, proto);
			if (multiinfo->ports[i-1] >= multiinfo->ports[i])
				exit_error(PARAMETER_PROBLEM,
					   "invalid portrange specified");
			m <<= 1;
		}
 	}
	multiinfo->count = i;
 	if (cp) exit_error(PARAMETER_PROBLEM, "too many ports specified");
 	free(buffer);
}

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
}

static const char *
check_proto(const struct ipt_entry *entry)
{
	char *proto;

	if (entry->ip.invflags & IPT_INV_PROTO)
		exit_error(PARAMETER_PROBLEM,
			   "multiport only works with TCP, UDP, UDPLITE, SCTP and DCCP");

	if ((proto = proto_to_name(entry->ip.proto)) != NULL)
		return proto;
	else if (!entry->ip.proto)
		exit_error(PARAMETER_PROBLEM,
			   "multiport needs `-p tcp', `-p udp', `-p udplite', "
			   "`-p sctp' or `-p dccp'");
	else
		exit_error(PARAMETER_PROBLEM,
			   "multiport only works with TCP, UDP, UDPLITE, SCTP and DCCP");
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse_v1(int c, char **argv, int invert, unsigned int *flags,
	 const struct ipt_entry *entry,
	 unsigned int *nfcache,
	 struct ipt_entry_match **match)
{
	const char *proto;
	struct ipt_multiport_v1 *multiinfo
		= (struct ipt_multiport_v1 *)(*match)->data;

	switch (c) {
	case '1':
		check_inverse(argv[optind-1], &invert, &optind, 0);
		proto = check_proto(entry);
		parse_multi_ports_v1(argv[optind-1], multiinfo, proto);
		multiinfo->flags = IPT_MULTIPORT_SOURCE;
		break;

	case '2':
		check_inverse(argv[optind-1], &invert, &optind, 0);
		proto = check_proto(entry);
		parse_multi_ports_v1(argv[optind-1], multiinfo, proto);
		multiinfo->flags = IPT_MULTIPORT_DESTINATION;
		break;

	case '3':
		check_inverse(argv[optind-1], &invert, &optind, 0);
		proto = check_proto(entry);
		parse_multi_ports_v1(argv[optind-1], multiinfo, proto);
		multiinfo->flags = IPT_MULTIPORT_EITHER;
		break;

	default:
		return 0;
	}

	if (invert)
		multiinfo->invert = 1;

	if (*flags)
		exit_error(PARAMETER_PROBLEM,
			   "multiport can only have one option");
	*flags = 1;
	return 1;
}

/* Final check; must specify something. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM, "multiport expection an option");
}

static char *
port_to_service(int port, u_int8_t proto)
{
	struct servent *service;

	if ((service = getservbyport(htons(port), proto_to_name(proto))))
		return service->s_name;

	return NULL;
}

static void
print_port(u_int16_t port, u_int8_t protocol, int numeric)
{
	char *service;

	if (numeric || (service = port_to_service(port, protocol)) == NULL)
		printf("%u", port);
	else
		printf("%s", service);
}

/* Prints out the matchinfo. */
static void
print_v1(const struct ipt_ip *ip,
	 const struct ipt_entry_match *match,
	 int numeric)
{
	const struct ipt_multiport_v1 *multiinfo
		= (const struct ipt_multiport_v1 *)match->data;
	unsigned int i;

	printf("multiport ");

	switch (multiinfo->flags) {
	case IPT_MULTIPORT_SOURCE:
		printf("sports ");
		break;

	case IPT_MULTIPORT_DESTINATION:
		printf("dports ");
		break;

	case IPT_MULTIPORT_EITHER:
		printf("ports ");
		break;

	default:
		printf("ERROR ");
		break;
	}

	if (multiinfo->invert)
		printf("! ");

	for (i=0; i < multiinfo->count; i++) {
		printf("%s", i ? "," : "");
		print_port(multiinfo->ports[i], ip->proto, numeric);
		if (multiinfo->pflags[i]) {
			printf(":");
			print_port(multiinfo->ports[++i], ip->proto, numeric);
		}
	}
	printf(" ");
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void save_v1(const struct ipt_ip *ip, 
		    const struct ipt_entry_match *match)
{
	const struct ipt_multiport_v1 *multiinfo
		= (const struct ipt_multiport_v1 *)match->data;
	unsigned int i;

	switch (multiinfo->flags) {
	case IPT_MULTIPORT_SOURCE:
		printf("--sports ");
		break;

	case IPT_MULTIPORT_DESTINATION:
		printf("--dports ");
		break;

	case IPT_MULTIPORT_EITHER:
		printf("--ports ");
		break;
	}

	if (multiinfo->invert)
		printf("! ");

	for (i=0; i < multiinfo->count; i++) {
		printf("%s", i ? "," : "");
		print_port(multiinfo->ports[i], ip->proto, 1);
		if (multiinfo->pflags[i]) {
			printf(":");
			print_port(multiinfo->ports[++i], ip->proto, 1);
		}
	}
	printf(" ");
}

static struct iptables_match multiport_v1 = { 
	.next		= NULL,
	.name		= "multiport",
	.version	= IPTABLES_VERSION,
	.revision	= 1,
	.size		= IPT_ALIGN(sizeof(struct ipt_multiport_v1)),
	.userspacesize	= IPT_ALIGN(sizeof(struct ipt_multiport_v1)),
	.help		= &help_v1,
	.init		= &init,
	.parse		= &parse_v1,
	.final_check	= &final_check,
	.print		= &print_v1,
	.save		= &save_v1,
	.extra_opts	= opts
};

void
_init(void)
{
	register_match(&multiport_v1);
}
