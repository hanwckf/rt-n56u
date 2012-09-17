/* Shared library add-on to iptables to add IP address pool matching. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#include <iptables.h>
//#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ipt_pool.h>

#include <libippool/ip_pool_support.h>

/* FIXME --RR */
#include "../ippool/libippool.c"

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"pool v%s options:\n"
" [!] --srcpool NAME|INDEX\n"
" [!] --dstpool NAME|INDEX\n"
"			Pool index (or name from %s) to match\n"
"\n", IPTABLES_VERSION, IPPOOL_CONF);
}

static struct option opts[] = {
	{ "srcpool", 1, 0, '1' },
	{ "dstpool", 1, 0, '2' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *match, unsigned int *nfcache)
{
	struct ipt_pool_info *info =
		(struct ipt_pool_info *)match->data;

	info->src = IP_POOL_NONE;
	info->dst = IP_POOL_NONE;
	info->flags = 0;
	/* Can't cache this - XXX */
	*nfcache |= NFC_UNKNOWN;
}

/* Function which parses command options; returns true if it ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_pool_info *info =
		(struct ipt_pool_info *)(*match)->data;

	switch (c) {
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		info->src = ip_pool_get_index(argv[optind-1]);
		if (invert) info->flags |= IPT_POOL_INV_SRC;
		*flags = 1;
		break;
	case '2':
		check_inverse(optarg, &invert, &optind, 0);
		info->dst = ip_pool_get_index(argv[optind-1]);
		if (invert) info->flags |= IPT_POOL_INV_DST;
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

/* Final check; must have specified --srcpool or --dstpool. */
static void final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM, "You must specify either `--srcpool or --dstpool'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	char buf[256];
	struct ipt_pool_info *info =
		(struct ipt_pool_info *)match->data;

	if (info->src != IP_POOL_NONE)
		printf("%ssrcpool %s ",
			(info->flags & IPT_POOL_INV_SRC) ? "!" : "",
			ip_pool_get_name(buf, sizeof(buf), info->src, 0));
	if (info->dst != IP_POOL_NONE)
		printf("%sdstpool %s ",
			(info->flags & IPT_POOL_INV_DST) ? "!" : "",
			ip_pool_get_name(buf, sizeof(buf), info->dst, 0));
}

/* Saves the matchinfo in parsable form to stdout. */
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	char buf[256];
	struct ipt_pool_info *info =
		(struct ipt_pool_info *)match->data;

	if (info->src != IP_POOL_NONE)
		printf("%s--srcpool %s",
			(info->flags & IPT_POOL_INV_SRC) ? "! " : "",
			ip_pool_get_name(buf, sizeof(buf), info->src, 0));
	if (info->dst != IP_POOL_NONE)
		printf("%s--dstpool %s",
			(info->flags & IPT_POOL_INV_DST) ? "! " : "",
			ip_pool_get_name(buf, sizeof(buf), info->dst, 0));
}

static
struct iptables_match pool = {
    .next          =    NULL,
    .name          =    "pool",
    .version       =    IPTABLES_VERSION,
    .size          =    IPT_ALIGN(sizeof(struct ipt_pool_info)),
    .userspacesize =    IPT_ALIGN(sizeof(struct ipt_pool_info)),
    .help          =    &help,
    .init          =    &init,
    .parse         =    &parse,
    .final_check   =    &final_check,
    .print         =    &print,
    .save          =    &save,
    .extra_opts    =    opts
};

void _init(void)
{
	register_match(&pool);
}
