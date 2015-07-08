/*
 *             Pseudo-driver for the intermediate queue device.
 *
 *             This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License
 *             as published by the Free Software Foundation; either version
 *             2 of the License, or (at your option) any later version.
 *
 * Authors:    Patrick McHardy, <kaber@trash.net>
 *
 *            The first version was written by Martin Devera, <devik@cdi.cz>
 *
 * Credits:    Jan Rafaj <imq2t@cedric.vabo.cz>
 *              - Update patch to 2.4.21
 *             Sebastian Strollo <sstrollo@nortelnetworks.com>
 *              - Fix "Dead-loop on netdevice imq"-issue
 *             Marcel Sebek <sebek64@post.cz>
 *              - Update to 2.6.2-rc1
 *
 *	       After some time of inactivity there is a group taking care
 *	       of IMQ again: http://www.linuximq.net
 *
 *
 *	       2004/06/30 - New version of IMQ patch to kernels <=2.6.7
 *             including the following changes:
 *
 *	       - Correction of ipv6 support "+"s issue (Hasso Tepper)
 *	       - Correction of imq_init_devs() issue that resulted in
 *	       kernel OOPS unloading IMQ as module (Norbert Buchmuller)
 *	       - Addition of functionality to choose number of IMQ devices
 *	       during kernel config (Andre Correa)
 *	       - Addition of functionality to choose how IMQ hooks on
 *	       PRE and POSTROUTING (after or before NAT) (Andre Correa)
 *	       - Cosmetic corrections (Norbert Buchmuller) (Andre Correa)
 *
 *
 *             2005/12/16 - IMQ versions between 2.6.7 and 2.6.13 were
 *             released with almost no problems. 2.6.14-x was released
 *             with some important changes: nfcache was removed; After
 *             some weeks of trouble we figured out that some IMQ fields
 *             in skb were missing in skbuff.c - skb_clone and copy_skb_header.
 *             These functions are correctly patched by this new patch version.
 *
 *             Thanks for all who helped to figure out all the problems with
 *             2.6.14.x: Patrick McHardy, Rune Kock, VeNoMouS, Max CtRiX,
 *             Kevin Shanahan, Richard Lucassen, Valery Dachev (hopefully
 *             I didn't forget anybody). I apologize again for my lack of time.
 *
 *
 *             2008/06/17 - 2.6.25 - Changed imq.c to use qdisc_run() instead
 *             of qdisc_restart() and moved qdisc_run() to tasklet to avoid
 *             recursive locking. New initialization routines to fix 'rmmod' not
 *             working anymore. Used code from ifb.c. (Jussi Kivilinna)
 *
 *             2008/08/06 - 2.6.26 - (JK)
 *              - Replaced tasklet with 'netif_schedule()'.
 *              - Cleaned up and added comments for imq_nf_queue().
 *
 *             2009/04/12
 *              - Add skb_save_cb/skb_restore_cb helper functions for backuping
 *                control buffer. This is needed because qdisc-layer on kernels
 *                2.6.27 and newer overwrite control buffer. (Jussi Kivilinna)
 *              - Add better locking for IMQ device. Hopefully this will solve
 *                SMP issues. (Jussi Kivilinna)
 *              - Port to 2.6.27
 *              - Port to 2.6.28
 *              - Port to 2.6.29 + fix rmmod not working
 *
 *             2009/04/20 - (Jussi Kivilinna)
 *              - Use netdevice feature flags to avoid extra packet handling
 *                by core networking layer and possibly increase performance.
 *
 *             2009/09/26 - (Jussi Kivilinna)
 *              - Add imq_nf_reinject_lockless to fix deadlock with
 *                imq_nf_queue/imq_nf_reinject.
 *
 *             2009/12/08 - (Jussi Kivilinna)
 *              - Port to 2.6.32
 *              - Add check for skb->nf_queue_entry==NULL in imq_dev_xmit()
 *              - Also add better error checking for skb->nf_queue_entry usage
 *
 *             2010/02/25 - (Jussi Kivilinna)
 *              - Port to 2.6.33
 *
 *             2010/08/15 - (Jussi Kivilinna)
 *              - Port to 2.6.35
 *              - Simplify hook registration by using nf_register_hooks.
 *              - nf_reinject doesn't need spinlock around it, therefore remove
 *                imq_nf_reinject function. Other nf_reinject users protect
 *                their own data with spinlock. With IMQ however all data is
 *                needed is stored per skbuff, so no locking is needed.
 *              - Changed IMQ to use 'separate' NF_IMQ_QUEUE instead of
 *                NF_QUEUE, this allows working coexistance of IMQ and other
 *                NF_QUEUE users.
 *              - Make IMQ multi-queue. Number of IMQ device queues can be
 *                increased with 'numqueues' module parameters. Default number
 *                of queues is 1, in other words by default IMQ works as
 *                single-queue device. Multi-queue selection is based on
 *                IFB multi-queue patch by Changli Gao <xiaosuo@gmail.com>.
 *
 *             2011/03/18 - (Jussi Kivilinna)
 *              - Port to 2.6.38
 *
 *             2011/07/12 - (syoder89@gmail.com)
 *              - Crash fix that happens when the receiving interface has more
 *                than one queue (add missing skb_set_queue_mapping in
 *                imq_select_queue).
 *
 *             2011/07/26 - (Jussi Kivilinna)
 *              - Add queue mapping checks for packets exiting IMQ.
 *              - Port to 3.0
 *
 *             2011/08/16 - (Jussi Kivilinna)
 *              - Clear IFF_TX_SKB_SHARING flag that was added for linux 3.0.2
 *
 *             2011/11/03 - Germano Michel <germanomichel@gmail.com>
 *              - Fix IMQ for net namespaces
 *
 *             2011/11/04 - Jussi Kivilinna <jussi.kivilinna@mbnet.fi>
 *              - Port to 3.1
 *              - Clean-up, move 'get imq device pointer by imqX name' to
 *                separate function from imq_nf_queue().
 *
 *             2012/01/05 - Jussi Kivilinna <jussi.kivilinna@mbnet.fi>
 *              - Port to 3.2
 *
 *             2012/03/19 - Jussi Kivilinna <jussi.kivilinna@mbnet.fi>
 *              - Port to 3.3
 *
 *	       Also, many thanks to pablo Sebastian Greco for making the initial
 *	       patch and to those who helped the testing.
 *
 *             More info at: http://www.linuximq.net/ (Andre Correa)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	#include <linux/netfilter_ipv6.h>
#endif
#include <linux/imq.h>
#include <net/pkt_sched.h>
#include <net/netfilter/nf_queue.h>
#include <net/sock.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <net/ip.h>
#include <net/ipv6.h>

static int imq_nf_queue(struct nf_queue_entry *entry, unsigned int queue_num);

static nf_hookfn imq_nf_hook;

static struct nf_hook_ops imq_ops[] = {
	{
	/* imq_ingress_ipv4 */
		.hook		= imq_nf_hook,
		.owner		= THIS_MODULE,
		.pf		= PF_INET,
		.hooknum	= NF_INET_PRE_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_BA) || defined(CONFIG_IMQ_BEHAVIOR_BB)
		.priority	= NF_IP_PRI_MANGLE + 1,
#else
		.priority	= NF_IP_PRI_NAT_DST + 1,
#endif
	},
	{
	/* imq_egress_ipv4 */
		.hook		= imq_nf_hook,
		.owner		= THIS_MODULE,
		.pf		= PF_INET,
		.hooknum	= NF_INET_POST_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_AA) || defined(CONFIG_IMQ_BEHAVIOR_BA)
		.priority	= NF_IP_PRI_LAST,
#else
		.priority	= NF_IP_PRI_NAT_SRC - 1,
#endif
	},
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	{
	/* imq_ingress_ipv6 */
		.hook		= imq_nf_hook,
		.owner		= THIS_MODULE,
		.pf		= PF_INET6,
		.hooknum	= NF_INET_PRE_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_BA) || defined(CONFIG_IMQ_BEHAVIOR_BB)
		.priority	= NF_IP6_PRI_MANGLE + 1,
#else
		.priority	= NF_IP6_PRI_NAT_DST + 1,
#endif
	},
	{
	/* imq_egress_ipv6 */
		.hook		= imq_nf_hook,
		.owner		= THIS_MODULE,
		.pf		= PF_INET6,
		.hooknum	= NF_INET_POST_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_AA) || defined(CONFIG_IMQ_BEHAVIOR_BA)
		.priority	= NF_IP6_PRI_LAST,
#else
		.priority	= NF_IP6_PRI_NAT_SRC - 1,
#endif
	},
#endif
};

#if defined(CONFIG_IMQ_NUM_DEVS)
static int numdevs = CONFIG_IMQ_NUM_DEVS;
#else
static int numdevs = IMQ_MAX_DEVS;
#endif

static struct net_device *imq_devs_cache[IMQ_MAX_DEVS];

#define IMQ_MAX_QUEUES 32
static int numqueues = 1;
static u32 imq_hashrnd;

static inline __be16 pppoe_proto(const struct sk_buff *skb)
{
	return *((__be16 *)(skb_mac_header(skb) + ETH_HLEN +
			sizeof(struct pppoe_hdr)));
}

static u16 imq_hash(struct net_device *dev, struct sk_buff *skb)
{
	unsigned int pull_len;
	u16 protocol = skb->protocol;
	u32 addr1, addr2;
	u32 hash, ihl = 0;
	union {
		u16 in16[2];
		u32 in32;
	} ports;
	u8 ip_proto;

	pull_len = 0;

recheck:
	switch (protocol) {
	case htons(ETH_P_8021Q): {
		if (unlikely(skb_pull(skb, VLAN_HLEN) == NULL))
			goto other;

		pull_len += VLAN_HLEN;
		skb->network_header += VLAN_HLEN;

		protocol = vlan_eth_hdr(skb)->h_vlan_encapsulated_proto;
		goto recheck;
	}

	case htons(ETH_P_PPP_SES): {
		if (unlikely(skb_pull(skb, PPPOE_SES_HLEN) == NULL))
			goto other;

		pull_len += PPPOE_SES_HLEN;
		skb->network_header += PPPOE_SES_HLEN;

		protocol = pppoe_proto(skb);
		goto recheck;
	}

	case htons(ETH_P_IP): {
		const struct iphdr *iph = ip_hdr(skb);

		if (unlikely(!pskb_may_pull(skb, sizeof(struct iphdr))))
			goto other;

		addr1 = iph->daddr;
		addr2 = iph->saddr;

		ip_proto = !(ip_hdr(skb)->frag_off & htons(IP_MF | IP_OFFSET)) ?
				 iph->protocol : 0;
		ihl = ip_hdrlen(skb);

		break;
	}
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	case htons(ETH_P_IPV6): {
		const struct ipv6hdr *iph = ipv6_hdr(skb);
		__be16 fo = 0;

		if (unlikely(!pskb_may_pull(skb, sizeof(struct ipv6hdr))))
			goto other;

		addr1 = iph->daddr.s6_addr32[3];
		addr2 = iph->saddr.s6_addr32[3];
		ihl = ipv6_skip_exthdr(skb, sizeof(struct ipv6hdr), &ip_proto, &fo);
		if (unlikely(ihl < 0))
			goto other;

		break;
	}
#endif
	default:
other:
		if (pull_len != 0) {
			skb_push(skb, pull_len);
			skb->network_header -= pull_len;
		}

		return (u16)(ntohs(protocol) % dev->real_num_tx_queues);
	}

	if (addr1 > addr2)
		swap(addr1, addr2);

	switch (ip_proto) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_DCCP:
	case IPPROTO_ESP:
	case IPPROTO_AH:
	case IPPROTO_SCTP:
	case IPPROTO_UDPLITE: {
		if (likely(skb_copy_bits(skb, ihl, &ports.in32, 4) >= 0)) {
			if (ports.in16[0] > ports.in16[1])
				swap(ports.in16[0], ports.in16[1]);
			break;
		}
		/* fall-through */
	}
	default:
		ports.in32 = 0;
		break;
	}

	if (pull_len != 0) {
		skb_push(skb, pull_len);
		skb->network_header -= pull_len;
	}

	hash = jhash_3words(addr1, addr2, ports.in32, imq_hashrnd ^ ip_proto);

	return (u16)(((u64)hash * dev->real_num_tx_queues) >> 32);
}

static inline bool sk_tx_queue_recorded(struct sock *sk)
{
	return (sk_tx_queue_get(sk) >= 0);
}

static struct netdev_queue *imq_select_queue(struct net_device *dev,
						struct sk_buff *skb)
{
	u16 queue_index = 0;
	u32 hash;

	if (likely(dev->real_num_tx_queues == 1))
		goto out;

	/* IMQ can be receiving ingress or engress packets. */

	/* Check first for if rx_queue is set */
	if (skb_rx_queue_recorded(skb)) {
		queue_index = skb_get_rx_queue(skb);
		goto out;
	}

	/* Check if socket has tx_queue set */
	if (sk_tx_queue_recorded(skb->sk)) {
		queue_index = sk_tx_queue_get(skb->sk);
		goto out;
	}

	/* Try use socket hash */
	if (skb->sk && skb->sk->sk_hash) {
		hash = skb->sk->sk_hash;
		queue_index =
			(u16)(((u64)hash * dev->real_num_tx_queues) >> 32);
		goto out;
	}

	/* Generate hash from packet data */
	queue_index = imq_hash(dev, skb);

out:
	if (unlikely(queue_index >= dev->real_num_tx_queues))
		queue_index = (u16)((u32)queue_index % dev->real_num_tx_queues);

	skb_set_queue_mapping(skb, queue_index);
	return netdev_get_tx_queue(dev, queue_index);
}

static struct net_device_stats *imq_get_stats(struct net_device *dev)
{
	return &dev->stats;
}

/* called for packets kfree'd in qdiscs at places other than enqueue */
static void imq_skb_destructor(struct sk_buff *skb)
{
	struct nf_queue_entry *entry = skb->nf_queue_entry;

	skb->nf_queue_entry = NULL;

	if (entry) {
		nf_queue_entry_release_refs(entry);
		kfree(entry);
	}

	skb_restore_cb(skb); /* kfree backup */
}

static void imq_done_check_queue_mapping(struct sk_buff *skb,
					 struct net_device *dev)
{
	unsigned int queue_index;

	/* Don't let queue_mapping be left too large after exiting IMQ */
	if (likely(skb->dev != dev && skb->dev != NULL)) {
		queue_index = skb_get_queue_mapping(skb);
		if (unlikely(queue_index >= skb->dev->real_num_tx_queues)) {
			queue_index = (u16)((u32)queue_index %
						skb->dev->real_num_tx_queues);
			skb_set_queue_mapping(skb, queue_index);
		}
	} else {
		/* skb->dev was IMQ device itself or NULL, be on safe side and
		 * just clear queue mapping.
		 */
		skb_set_queue_mapping(skb, 0);
	}
}

static netdev_tx_t imq_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct nf_queue_entry *entry = skb->nf_queue_entry;

	skb->nf_queue_entry = NULL;
	dev->trans_start = jiffies;

	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets++;

	if (unlikely(entry == NULL)) {
		/* We don't know what is going on here.. packet is queued for
		 * imq device, but (probably) not by us.
		 *
		 * If this packet was not send here by imq_nf_queue(), then
		 * skb_save_cb() was not used and skb_free() should not show:
		 *   WARNING: IMQ: kfree_skb: skb->cb_next:..
		 * and/or
		 *   WARNING: IMQ: kfree_skb: skb->nf_queue_entry...
		 *
		 * However if this message is shown, then IMQ is somehow broken
		 * and you should report this to linuximq.net.
		 */

		/* imq_dev_xmit is black hole that eats all packets, report that
		 * we eat this packet happily and increase dropped counters.
		 */

		dev->stats.tx_dropped++;
		dev_kfree_skb(skb);

		return NETDEV_TX_OK;
	}

	skb_restore_cb(skb); /* restore skb->cb */

	skb->imq_flags = 0;
	skb->destructor = NULL;

	imq_done_check_queue_mapping(skb, dev);

	nf_reinject(entry, NF_ACCEPT);

	return NETDEV_TX_OK;
}

static struct net_device *get_imq_device_by_index(int index)
{
	struct net_device *dev = NULL;
	struct net *net;
	char buf[8];

	/* get device by name and cache result */
	snprintf(buf, sizeof(buf), "imq%d", index);

	/* Search device from all namespaces. */
	for_each_net(net) {
		dev = dev_get_by_name(net, buf);
		if (dev)
			break;
	}

	if (WARN_ON_ONCE(dev == NULL)) {
		/* IMQ device not found. Exotic config? */
		return ERR_PTR(-ENODEV);
	}

	imq_devs_cache[index] = dev;
	dev_put(dev);

	return dev;
}

static int imq_nf_queue(struct nf_queue_entry *entry, unsigned int queue_num)
{
	struct net_device *dev;
	struct sk_buff *skb_orig, *skb, *skb_shared;
	struct Qdisc *q;
	struct netdev_queue *txq;
	spinlock_t *root_lock;
	int users, index;
	int retval = -EINVAL;
	unsigned int orig_queue_index;

	index = entry->skb->imq_flags & IMQ_F_IFMASK;
	if (unlikely(index > numdevs - 1)) {
		if (net_ratelimit())
			printk(KERN_WARNING
			       "IMQ: invalid device specified, highest is %u\n",
			       numdevs - 1);
		retval = -EINVAL;
		goto out;
	}

	/* check for imq device by index from cache */
	dev = imq_devs_cache[index];
	if (unlikely(!dev)) {
		dev = get_imq_device_by_index(index);
		if (IS_ERR(dev)) {
			retval = PTR_ERR(dev);
			goto out;
		}
	}

	if (unlikely(!(dev->flags & IFF_UP))) {
		entry->skb->imq_flags = 0;
		nf_reinject(entry, NF_ACCEPT);
		retval = 0;
		goto out;
	}
	dev->last_rx = jiffies;

	skb = entry->skb;
	skb_orig = NULL;

	/* skb has owner? => make clone */
	if (unlikely(skb->destructor)) {
		skb_orig = skb;
		skb = skb_clone(skb, GFP_ATOMIC);
		if (unlikely(!skb)) {
			retval = -ENOMEM;
			goto out;
		}
		entry->skb = skb;
	}

	skb->nf_queue_entry = entry;

	dev->stats.rx_bytes += skb->len;
	dev->stats.rx_packets++;

	if (!skb->dev) {
		/* skb->dev == NULL causes problems, try the find cause. */
		if (net_ratelimit()) {
			dev_warn(&dev->dev,
				 "received packet with skb->dev == NULL\n");
			dump_stack();
		}

		skb->dev = dev;
	}

	/* Disables softirqs for lock below */
	rcu_read_lock_bh();

	/* Multi-queue selection */
	orig_queue_index = skb_get_queue_mapping(skb);
	txq = imq_select_queue(dev, skb);

	q = rcu_dereference(txq->qdisc);
	if (unlikely(!q->enqueue))
		goto packet_not_eaten_by_imq_dev;

	root_lock = qdisc_lock(q);
	spin_lock(root_lock);

	users = atomic_read(&skb->users);

	skb_shared = skb_get(skb); /* increase reference count by one */
	skb_save_cb(skb_shared); /* backup skb->cb, as qdisc layer will
					overwrite it */
	qdisc_enqueue_root(skb_shared, q); /* might kfree_skb */

	if (likely(atomic_read(&skb_shared->users) == users + 1)) {
		kfree_skb(skb_shared); /* decrease reference count by one */

		skb->destructor = &imq_skb_destructor;

		/* cloned? */
		if (unlikely(skb_orig))
			kfree_skb(skb_orig); /* free original */

		spin_unlock(root_lock);
		rcu_read_unlock_bh();

		/* schedule qdisc dequeue */
		__netif_schedule(q);

		retval = 0;
		goto out;
	} else {
		skb_restore_cb(skb_shared); /* restore skb->cb */
		skb->nf_queue_entry = NULL;
		/* qdisc dropped packet and decreased skb reference count of
		 * skb, so we don't really want to and try refree as that would
		 * actually destroy the skb. */
		spin_unlock(root_lock);
		goto packet_not_eaten_by_imq_dev;
	}

packet_not_eaten_by_imq_dev:
	skb_set_queue_mapping(skb, orig_queue_index);
	rcu_read_unlock_bh();

	/* cloned? restore original */
	if (unlikely(skb_orig)) {
		kfree_skb(skb);
		entry->skb = skb_orig;
	}
	retval = -1;
out:
	return retval;
}

static unsigned int imq_nf_hook(unsigned int hook, struct sk_buff *pskb,
				const struct net_device *indev,
				const struct net_device *outdev,
				int (*okfn)(struct sk_buff *))
{
	return (pskb->imq_flags & IMQ_F_ENQUEUE) ? NF_IMQ_QUEUE : NF_ACCEPT;
}

static int imq_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static int imq_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}

static const struct net_device_ops imq_netdev_ops = {
	.ndo_open		= imq_open,
	.ndo_stop		= imq_close,
	.ndo_start_xmit		= imq_dev_xmit,
	.ndo_get_stats		= imq_get_stats,
};

static void imq_setup(struct net_device *dev)
{
	dev->netdev_ops		= &imq_netdev_ops;
	dev->type		= ARPHRD_VOID;
	dev->mtu		= 16000; /* too small? */
	dev->tx_queue_len	= 11000; /* too big? */
	dev->flags		= IFF_NOARP;
	dev->features		= NETIF_F_SG | NETIF_F_FRAGLIST |
				  NETIF_F_GSO | NETIF_F_HW_CSUM |
				  NETIF_F_HIGHDMA;
	dev->priv_flags		&= ~(IFF_XMIT_DST_RELEASE |
				     IFF_TX_SKB_SHARING);
}

static int imq_validate(struct nlattr *tb[], struct nlattr *data[])
{
	int ret = 0;

	if (tb[IFLA_ADDRESS]) {
		if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN) {
			ret = -EINVAL;
			goto end;
		}
		if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS]))) {
			ret = -EADDRNOTAVAIL;
			goto end;
		}
	}
	return 0;
end:
	printk(KERN_WARNING "IMQ: imq_validate failed (%d)\n", ret);
	return ret;
}

static struct rtnl_link_ops imq_link_ops __read_mostly = {
	.kind		= "imq",
	.priv_size	= 0,
	.setup		= imq_setup,
	.validate	= imq_validate,
};

static const struct nf_queue_handler imq_nfqh = {
	.name  = "imq",
	.outfn = imq_nf_queue,
};

static int __init imq_init_hooks(void)
{
	int ret;

	nf_register_queue_imq_handler(&imq_nfqh);

	ret = nf_register_hooks(imq_ops, ARRAY_SIZE(imq_ops));
	if (ret < 0)
		nf_unregister_queue_imq_handler();

	return ret;
}

static int __init imq_init_one(int index)
{
	struct net_device *dev;
	int ret;

	dev = alloc_netdev_mq(0, "imq%d", imq_setup, numqueues);
	if (!dev)
		return -ENOMEM;

	ret = dev_alloc_name(dev, dev->name);
	if (ret < 0)
		goto fail;

	dev->rtnl_link_ops = &imq_link_ops;
	ret = register_netdevice(dev);
	if (ret < 0)
		goto fail;

	return 0;
fail:
	free_netdev(dev);
	return ret;
}

static int __init imq_init_devs(void)
{
	int err, i;

	if (numdevs < 1 || numdevs > IMQ_MAX_DEVS) {
		printk(KERN_ERR "IMQ: numdevs has to be betweed 1 and %u\n",
		       IMQ_MAX_DEVS);
		return -EINVAL;
	}

	if (numqueues < 1 || numqueues > IMQ_MAX_QUEUES) {
		printk(KERN_ERR "IMQ: numqueues has to be betweed 1 and %u\n",
		       IMQ_MAX_QUEUES);
		return -EINVAL;
	}

	get_random_bytes(&imq_hashrnd, sizeof(imq_hashrnd));

	rtnl_lock();
	err = __rtnl_link_register(&imq_link_ops);

	for (i = 0; i < numdevs && !err; i++)
		err = imq_init_one(i);

	if (err) {
		__rtnl_link_unregister(&imq_link_ops);
		memset(imq_devs_cache, 0, sizeof(imq_devs_cache));
	}
	rtnl_unlock();

	return err;
}

static int __init imq_init_module(void)
{
	int err;

#if defined(CONFIG_IMQ_NUM_DEVS)
	BUILD_BUG_ON(CONFIG_IMQ_NUM_DEVS > 16);
	BUILD_BUG_ON(CONFIG_IMQ_NUM_DEVS < 2);
	BUILD_BUG_ON(CONFIG_IMQ_NUM_DEVS - 1 > IMQ_F_IFMASK);
#endif

	err = imq_init_devs();
	if (err) {
		printk(KERN_ERR "IMQ: Error trying imq_init_devs(net)\n");
		return err;
	}

	err = imq_init_hooks();
	if (err) {
		printk(KERN_ERR "IMQ: Error trying imq_init_hooks()\n");
		rtnl_link_unregister(&imq_link_ops);
		memset(imq_devs_cache, 0, sizeof(imq_devs_cache));
		return err;
	}

	printk(KERN_INFO "IMQ driver loaded successfully. "
		"(numdevs = %d, numqueues = %d)\n", numdevs, numqueues);

#if defined(CONFIG_IMQ_BEHAVIOR_BA) || defined(CONFIG_IMQ_BEHAVIOR_BB)
	printk(KERN_INFO "\tHooking IMQ before NAT on PREROUTING.\n");
#else
	printk(KERN_INFO "\tHooking IMQ after NAT on PREROUTING.\n");
#endif
#if defined(CONFIG_IMQ_BEHAVIOR_AB) || defined(CONFIG_IMQ_BEHAVIOR_BB)
	printk(KERN_INFO "\tHooking IMQ before NAT on POSTROUTING.\n");
#else
	printk(KERN_INFO "\tHooking IMQ after NAT on POSTROUTING.\n");
#endif

	return 0;
}

static void __exit imq_unhook(void)
{
	nf_unregister_hooks(imq_ops, ARRAY_SIZE(imq_ops));
	nf_unregister_queue_imq_handler();
}

static void __exit imq_cleanup_devs(void)
{
	rtnl_link_unregister(&imq_link_ops);
	memset(imq_devs_cache, 0, sizeof(imq_devs_cache));
}

static void __exit imq_exit_module(void)
{
	imq_unhook();
	imq_cleanup_devs();
	printk(KERN_INFO "IMQ driver unloaded successfully.\n");
}

module_init(imq_init_module);
module_exit(imq_exit_module);

module_param(numdevs, int, 0);
module_param(numqueues, int, 0);
MODULE_PARM_DESC(numdevs, "number of IMQ devices (how many imq* devices will "
			"be created)");
MODULE_PARM_DESC(numqueues, "number of queues per IMQ device");
MODULE_AUTHOR("http://www.linuximq.net");
MODULE_DESCRIPTION("Pseudo-driver for the intermediate queue device. See "
			"http://www.linuximq.net/ for more information.");
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK("imq");

