/*
 *  Fastpath module for NAT speedup.
 *  This module write BCM LTD and use some GPLv2 code blocks.
 *  It grants the right to change the license on GPL.
 *  Some code clenup and rewrite - Tomato and Wive projects.
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/neighbour.h>
#include <net/netfilter/nf_conntrack_core.h>

//#define DEBUG

typedef int (*bcmNatHitHook)(struct sk_buff *skb);
typedef int (*bcmNatBindHook)(struct nf_conn *ct, enum ip_conntrack_info ctinfo, unsigned int hooknum, struct sk_buff **pskb);

extern int bcm_nat_hit_hook_func(bcmNatHitHook hook_func);
extern int bcm_nat_bind_hook_func(bcmNatBindHook hook_func);

extern inline int
manip_pkt(u_int16_t proto,
	  struct sk_buff **pskb,
	  unsigned int iphdroff,
	  const struct nf_conntrack_tuple *target,
	  enum nf_nat_manip_type maniptype);

/*
 * check NAT session initialized and ready
 */
static inline int nat_is_ready(struct nf_conn *ct)
{
	/* If NAT initialized is finished may be offload */
	if ((ct->status & IPS_NAT_DONE_MASK) == IPS_NAT_DONE_MASK)
		return 1;
	return 0;
}

/*
 * check SKB really accesseble
 */
static inline int skb_is_ready(struct sk_buff *skb)
{
	if (skb_cloned(skb) && !skb->sk)
		return 0;
	return 1;
}

/*
 * Direct send packets to output.
 * Stolen from ip_finish_output2.
 */
static inline int bcm_fast_path_output(struct sk_buff *skb)
{
	struct dst_entry *dst = skb->dst;
	struct hh_cache *hh = dst->hh;
	int ret = 0;

	if (hh)
	    ret = neigh_hh_output(hh, skb);
	else if (dst->neighbour)
		ret = dst->neighbour->output(skb);

	/* Don't return 1 */
	if (ret == 1)
	    return 0;

	return ret;
}

static inline int bcm_fast_path(struct sk_buff *skb)
{
	if (skb->dst == NULL) {
		struct iphdr *iph = ip_hdr(skb);
		struct net_device *dev = skb->dev;

		if (ip_route_input(skb, iph->daddr, iph->saddr, iph->tos, dev)) {
			return NF_DROP;
		}
		/*  Change skb owner to output device */
		skb->dev = skb->dst->dev;
	}

	if (skb->dst) {
		if (skb->len > ip_skb_dst_mtu(skb) && !skb_is_gso(skb))
			return ip_fragment(skb, bcm_fast_path_output);
		else
			return bcm_fast_path_output(skb);
	}

#ifdef DEBUG
	if (net_ratelimit())
		printk(KERN_DEBUG "bcm_fast_path: No header cache and no neighbour!\n");
#endif

	kfree_skb(skb);
	return -EINVAL;
}

static inline int
bcm_do_bindings(struct nf_conn *ct,
		enum ip_conntrack_info ctinfo,
		struct sk_buff **pskb,
		struct nf_conntrack_l3proto *l3proto,
		struct nf_conntrack_l4proto *l4proto)
{
	struct iphdr *iph = ip_hdr(*pskb);
	static int hn[2] = {NF_IP_PRE_ROUTING, NF_IP_POST_ROUTING};
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	unsigned int i = 1;

	/* This check prevent corrupt conntrack data */
	if(!nat_is_ready(ct) || !skb_is_ready(*pskb)) {
#ifdef DEBUG
		if (net_ratelimit())
		    printk(KERN_DEBUG "bcm_fast_path: SKB or CT not ready for offload\n");
#endif
		return NF_ACCEPT; /* Ignore */
	}

	do {
		enum nf_nat_manip_type mtype = HOOK2MANIP(hn[i]);
		unsigned long statusbit;

		if (mtype == IP_NAT_MANIP_SRC)
			statusbit = IPS_SRC_NAT;
		else
			statusbit = IPS_DST_NAT;

		/* Invert if this is reply dir. */
		if (dir == IP_CT_DIR_REPLY)
			statusbit ^= IPS_NAT_MASK;

		if (ct->status & statusbit) {
			struct nf_conntrack_tuple target;

			if ((*pskb)->dst == NULL && mtype == IP_NAT_MANIP_SRC) {
				struct net_device *dev = (*pskb)->dev;
				if (ip_route_input((*pskb), iph->daddr, iph->saddr, iph->tos, dev))
					return NF_DROP;
				/* Change skb owner */
				(*pskb)->dev = (*pskb)->dst->dev;
			}

			/* We are aiming to look like inverse of other direction. */
			nf_ct_invert_tuple(&target, &ct->tuplehash[!dir].tuple, l3proto, l4proto);

			if (!manip_pkt(target.dst.protonum, pskb, 0, &target, mtype))
				return NF_DROP;
		}
	} while (i++ < 2);

	return NF_FAST_NAT;
}

static int __init bcm_nat_init(void)
{
	bcm_nat_hit_hook_func (bcm_fast_path);
	bcm_nat_bind_hook_func ((bcmNatBindHook)bcm_do_bindings);
	printk("NAT Fastpath init.\n");
	return 0;
}

static void __exit bcm_nat_fini(void)
{
	bcm_nat_hit_hook_func (NULL);
	bcm_nat_bind_hook_func (NULL);
}

module_init(bcm_nat_init);
module_exit(bcm_nat_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Broadcom fastpath module for NAT offload.\n");
