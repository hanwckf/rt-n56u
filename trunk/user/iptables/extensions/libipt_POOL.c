/* Shared library add-on to iptables to add IP pool mangling target. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_pool.h>
#include <linux/netfilter_ipv4/ipt_pool.h>

#include <libippool/ip_pool_support.h>

/* FIXME --RR */
#define ip_pool_init           ip_POOL_init
#define ip_pool_get_index      ip_POOL_get_index
#define ip_pool_get_name       ip_POOL_get_name
#include "../ippool/libippool.c"

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"POOL v%s options:\n"
" --add-srcip <pool>\n"
" --del-srcip <pool>\n"
" --add-dstip <pool>\n"
" --del-dstip <pool>\n"
"				add/del src/dst IP from pool.\n\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "add-srcip", 1, 0, '1' },
	{ "del-srcip", 1, 0, '2' },
	{ "add-dstip", 1, 0, '3' },
	{ "del-dstip", 1, 0, '4' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *target, unsigned int *nfcache)
{
	struct ipt_pool_info *ipi = (struct ipt_pool_info *) target->data;

	ipi->src = ipi->dst = IP_POOL_NONE;
	ipi->flags = 0;

	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_pool_info *ipi = (struct ipt_pool_info *) (*target)->data;
	switch (c) {
	case '1':	/* --add-srcip <pool> */
		ipi->src = ip_pool_get_index(optarg);
		ipi->flags &= ~IPT_POOL_DEL_SRC;
		break;
	case '2':	/* --del-srcip <pool> */
		ipi->src = ip_pool_get_index(optarg);
		ipi->flags |= IPT_POOL_DEL_SRC;
		break;
	case '3':	/* --add-dstip <pool> */
		ipi->dst = ip_pool_get_index(optarg);
		ipi->flags &= ~IPT_POOL_DEL_DST;
		break;
	case '4':	/* --del-dstip <pool> */
		ipi->dst = ip_pool_get_index(optarg);
		ipi->flags |= IPT_POOL_DEL_DST;
		break;
	default:
		return 0;
	}
	return 1;
}

/* Final check; don't care. */
static void final_check(unsigned int flags)
{
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	char buf[256];
	struct ipt_pool_info *ipi = (struct ipt_pool_info *) target->data;

	printf("POOL");
	if (ipi->src != IP_POOL_NONE) {
		printf(" --%s-srcip %s",
			(ipi->flags & IPT_POOL_DEL_SRC) ? "del" : "add",
			ip_pool_get_name(buf, sizeof(buf), ipi->src, numeric));
	}
	if (ipi->dst != IP_POOL_NONE) {
		printf(" --%s-dstip %s",
			(ipi->flags & IPT_POOL_DEL_DST) ? "del" : "add",
			ip_pool_get_name(buf, sizeof(buf), ipi->dst, numeric));
	}
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	char buf[256];
	struct ipt_pool_info *ipi = (struct ipt_pool_info *) target->data;

	printf("-j POOL");
	if (ipi->src != IP_POOL_NONE) {
		printf(" --%s-srcip %s",
			(ipi->flags & IPT_POOL_DEL_SRC) ? "del" : "add",
			ip_pool_get_name(buf, sizeof(buf), ipi->src, 0));
	}
	if (ipi->dst != IP_POOL_NONE) {
		printf(" --%s-dstip %s",
			(ipi->flags & IPT_POOL_DEL_DST) ? "del" : "add",
			ip_pool_get_name(buf, sizeof(buf), ipi->dst, 0));
	}
}

static
struct iptables_target ipt_pool_target = {
    .next          =    NULL,
    .name          =    "POOL",
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
	register_target(&ipt_pool_target);
}
