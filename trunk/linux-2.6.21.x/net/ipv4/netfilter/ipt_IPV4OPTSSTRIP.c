/**
 * Strip all IP options in the IP packet header.
 *
 * (C) 2001 by Fabrice MARIE <fabrice@netfilter.org>
 * This software is distributed under GNU GPL v2, 1991
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/checksum.h>

#include <linux/netfilter_ipv4/ip_tables.h>

MODULE_AUTHOR("Fabrice MARIE <fabrice@netfilter.org>");
MODULE_DESCRIPTION("Strip all options in IPv4 packets");
MODULE_LICENSE("GPL");

static unsigned int
target(struct sk_buff **pskb,
       const struct net_device *in,
       const struct net_device *out,
       unsigned int hooknum,
       const void *targinfo,
       void *userinfo)
{
	struct iphdr *iph;
	struct sk_buff *skb;
	struct ip_options *opt;
	unsigned char *optiph;
	int l;

	skb = (*pskb);
	iph = (*pskb)->nh.iph;
	optiph = skb->nh.raw;
	l = ((struct ip_options *)(&(IPCB(skb)->opt)))->optlen;

	/* if no options in packet then nothing to clear. */
	if (iph->ihl * 4 == sizeof(struct iphdr))
		return IPT_CONTINUE;

	/* else clear all options */
	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
	memset(optiph+sizeof(struct iphdr), IPOPT_NOOP, l);
	opt = &(IPCB(skb)->opt);
	opt->is_data = 0;
	opt->optlen = l;

        return IPT_CONTINUE;
}

static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
           void *targinfo,
           unsigned int targinfosize,
           unsigned int hook_mask)
{
	if (strcmp(tablename, "mangle")) {
		printk(KERN_WARNING "IPV4OPTSSTRIP: can only be called from \"mangle\" table, not \"%s\"\n", tablename);
		return 0;
	}
	/* nothing else to check because no parameters */
	return 1;
}

static struct xt_target xt_ipv4optsstrip_reg = { 
	.name = "IPV4OPTSSTRIP",
	.target = target,
	.checkentry = checkentry,
	.me = THIS_MODULE };

static int __init init(void)
{
	return xt_register_target(&xt_ipv4optsstrip_reg);
}

static void __exit fini(void)
{
	xt_unregister_target(&xt_ipv4optsstrip_reg);
}

module_init(init);
module_exit(fini);
