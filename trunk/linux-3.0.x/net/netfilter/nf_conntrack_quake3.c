/* Quake3 extension for IP connection tracking
 * (C) 2002 by Filip Sneppe <filip.sneppe@cronos.be>
 * (C) 2005 by Harald Welte <laforge@netfilter.org>
 * based on nf_conntrack_ftp.c and nf_conntrack_tftp.c
 *
 * nf_conntrack_quake3.c v0.04 2002-08-31
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *      Module load syntax:
 *      insmod nf_conntrack_quake3.o ports=port1,port2,...port<MAX_PORTS>
 *
 *      please give the ports of all Quake3 master servers You wish to
 *      connect to. If you don't specify ports, the default will be UDP
 *      port 27950.
 *
 *      Thanks to the Ethereal folks for their analysis of the Quake3 protocol.
 */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_quake3.h>

MODULE_AUTHOR("Filip Sneppe <filip.sneppe@cronos.be>");
MODULE_DESCRIPTION("Netfilter connection tracking module for Quake III Arena");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ip_conntrack_quake3");

unsigned int (*nf_nat_quake3_hook)(struct sk_buff *skb,
				   enum ip_conntrack_info ctinfo,
				   struct nf_conntrack_expect *exp)
				   __read_mostly;
EXPORT_SYMBOL_GPL(nf_nat_quake3_hook);


#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int ports_c = 0;
module_param_array(ports, int, &ports_c, 0400);
MODULE_PARM_DESC(ports, "port numbers of Quake III master servers");

static char quake3_buffer[65536];
static DEFINE_SPINLOCK(quake3_buffer_lock);

/* Quake3 master server reply will add > 100 expectations per reply packet; when
   doing lots of printk's, klogd may not be able to read /proc/kmsg fast enough */

struct quake3_search quake3s_conntrack = { "****", "getserversResponse", sizeof("getserversResponse") - 1 };

static int quake3_help(struct sk_buff *skb,
	unsigned int protoff,
	struct nf_conn *ct,
	enum ip_conntrack_info ctinfo)
{
	struct udphdr _udph, *uh;
	struct nf_conntrack_expect *exp;
	void *data, *qb_ptr;
	int dir = CTINFO2DIR(ctinfo);
	int i, dataoff;
	int ret = NF_ACCEPT;
	typeof(nf_nat_quake3_hook) nf_nat_quake3;

	/* Until there's been traffic both ways, don't look in packets. note:
	 * it's UDP ! */
	if (ctinfo != IP_CT_ESTABLISHED
	    && ctinfo != IP_CT_IS_REPLY) {
	        pr_debug("nf_conntrack_quake3: not ok ! Conntrackinfo = %u\n",
			ctinfo);
	        return NF_ACCEPT;
	} else {
		pr_debug("nf_conntrack_quake3: it's ok ! Conntrackinfo = %u\n",
			ctinfo);
	}

	/* Valid UDP header? */
	uh = skb_header_pointer(skb, protoff, sizeof(_udph), &_udph);
	if (!uh)
		return NF_ACCEPT;

	/* Any data? */
	dataoff = protoff + sizeof(struct udphdr);
	if (dataoff >= skb->len)
		return NF_ACCEPT;

	spin_lock_bh(&quake3_buffer_lock);
	qb_ptr = skb_header_pointer(skb, dataoff,
				    skb->len - dataoff, quake3_buffer);
	BUG_ON(qb_ptr == NULL);
	data = qb_ptr;

	if (strnicmp(data + 4, quake3s_conntrack.pattern,
		     quake3s_conntrack.plen) == 0) {
		for(i=23;    /* 4 bytes filler, 18 bytes "getserversResponse",
				1 byte "\" */
		    i+6 < ntohs(uh->len);
		    i+=7) {
			u_int32_t *ip = data+i;
			u_int16_t *port = data+i+4;
#if 0
			pr_debug("nf_conntrack_quake3: adding server at offset "
			       "%u/%u %u.%u.%u.%u:%u\n", i, ntohs(uh->len),
			       NIPQUAD(*ip), ntohs(*port));
#endif

			exp = nf_ct_expect_alloc(ct);
			if (!exp) {
				ret = NF_DROP;
				goto out;
			}

			memset(exp, 0, sizeof(*exp));

			exp->tuple.src.u3.ip = ct->tuplehash[!dir].tuple.src.u3.ip;
			exp->tuple.dst.u3.ip = *ip;
			exp->tuple.dst.u.udp.port = *port;
			exp->tuple.dst.protonum = IPPROTO_UDP;

			exp->mask.src.u3.ip = 0xffffffff;

			nf_nat_quake3 = rcu_dereference(nf_nat_quake3_hook);
			if (nf_nat_quake3 && ct->status & IPS_NAT_MASK)
				ret = nf_nat_quake3(skb, ctinfo, exp);
			else if (nf_ct_expect_related(exp) != 0)
				ret = NF_DROP;
			nf_ct_expect_put(exp);
			goto out;
		}
	}

out:
	spin_unlock_bh(&quake3_buffer_lock);

	return ret;
}

static struct nf_conntrack_helper quake3[MAX_PORTS] __read_mostly;
static char quake3_names[MAX_PORTS][sizeof("quake3-65535")] __read_mostly;

static const struct nf_conntrack_expect_policy quake_exp_policy = {
	.timeout	= 120,
};

static void fini(void)
{
	int i;

	for(i = 0; i < ports_c; i++)
			nf_conntrack_helper_unregister(&quake3[i]);
}

static int __init init(void)
{
	int i, ret;
	char *tmpname;

	if (ports_c == 0)
		ports[ports_c++] = QUAKE3_MASTER_PORT;

	for(i = 0; i < ports_c; i++) {
		/* Create helper structure */
		memset(&quake3[i], 0, sizeof(struct nf_conntrack_helper));
		quake3[i].tuple.src.l3num = AF_INET;

		quake3[i].tuple.dst.protonum = IPPROTO_UDP;
		quake3[i].tuple.src.u.udp.port = htons(ports[i]);
		quake3[i].help = quake3_help;
		quake3[i].me = THIS_MODULE;
		quake3[i].expect_policy = &quake_exp_policy;

		tmpname = &quake3_names[i][0];
		if (ports[i] == QUAKE3_MASTER_PORT)
			sprintf(tmpname, "quake3");
		else
			sprintf(tmpname, "quake3-%d", i);
		quake3[i].name = tmpname;

		ret = nf_conntrack_helper_register(&quake3[i]);
		if(ret) {
			printk(KERN_ERR "nf_ct_quake: failed to register"
			       " helper for pf: %u port: %u\n",
				quake3[i].tuple.src.l3num, ports[i]);
			fini();
			return ret;
		}
	}

	return 0;
}

module_init(init);
module_exit(fini);
