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
 *	       2004/06/30 - New version of IMQ patch to kernels <=2.6.7 including
 *	       the following changes:
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
 *             More info at: http://www.linuximq.net/ (Andre Correa)
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/if_arp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
	#include <linux/netfilter_ipv6.h>
#endif
#include <linux/imq.h>
#include <net/pkt_sched.h>

extern int qdisc_restart1(struct net_device *dev);

static nf_hookfn imq_nf_hook;

static struct nf_hook_ops imq_ingress_ipv4 = {
	.hook		= imq_nf_hook,
	.owner		= THIS_MODULE,
	.pf		= PF_INET,
	.hooknum	= NF_IP_PRE_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_BA) || defined(CONFIG_IMQ_BEHAVIOR_BB)
	.priority	= NF_IP_PRI_MANGLE + 1
#else
	.priority	= NF_IP_PRI_NAT_DST + 1
#endif
};

static struct nf_hook_ops imq_egress_ipv4 = {
	.hook		= imq_nf_hook,
	.owner		= THIS_MODULE,
	.pf		= PF_INET,
	.hooknum	= NF_IP_POST_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_AA) || defined(CONFIG_IMQ_BEHAVIOR_BA)
	.priority	= NF_IP_PRI_LAST
#else
	.priority	= NF_IP_PRI_NAT_SRC - 1
#endif
};

#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
static struct nf_hook_ops imq_ingress_ipv6 = {
	.hook		= imq_nf_hook,
	.owner		= THIS_MODULE,
	.pf		= PF_INET6,
	.hooknum	= NF_IP6_PRE_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_BA) || defined(CONFIG_IMQ_BEHAVIOR_BB)
	.priority	= NF_IP6_PRI_MANGLE + 1
#else
	.priority	= NF_IP6_PRI_NAT_DST + 1
#endif
};

static struct nf_hook_ops imq_egress_ipv6 = {
	.hook		= imq_nf_hook,
	.owner		= THIS_MODULE,
	.pf		= PF_INET6,
	.hooknum	= NF_IP6_POST_ROUTING,
#if defined(CONFIG_IMQ_BEHAVIOR_AA) || defined(CONFIG_IMQ_BEHAVIOR_BA)
	.priority	= NF_IP6_PRI_LAST
#else
	.priority	= NF_IP6_PRI_NAT_SRC - 1
#endif
};
#endif

#if defined(CONFIG_IMQ_NUM_DEVS)
static int numdevs = CONFIG_IMQ_NUM_DEVS;
#else
static int numdevs = 2;
#endif

static struct net_device *imq_devs;

static struct net_device_stats *imq_get_stats(struct net_device *dev)
{
	return (struct net_device_stats *)dev->priv;
}

/* called for packets kfree'd in qdiscs at places other than enqueue */
static void imq_skb_destructor(struct sk_buff *skb)
{
	struct nf_info *info = skb->nf_info;

	if (info) {
		if (info->indev)
			dev_put(info->indev);
		if (info->outdev)
			dev_put(info->outdev);
		kfree(info);
	}
}

static int imq_dev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = (struct net_device_stats*) dev->priv;

	stats->tx_bytes += skb->len;
	stats->tx_packets++;

	skb->imq_flags = 0;
	skb->destructor = NULL;

	dev->trans_start = jiffies;
	nf_reinject(skb, skb->nf_info, NF_ACCEPT);
	return 0;
}

static int imq_nf_queue(struct sk_buff *skb, struct nf_info *info, unsigned queue_num, void *data)
{
	struct net_device *dev;
	struct net_device_stats *stats;
	struct sk_buff *skb2 = NULL;
	struct Qdisc *q;
	unsigned int index = skb->imq_flags&IMQ_F_IFMASK;
	int ret = -1;

	if (index > numdevs)
		return -1;

	dev = imq_devs + index;
	if (!(dev->flags & IFF_UP)) {
		skb->imq_flags = 0;
		nf_reinject(skb, info, NF_ACCEPT);
		return 0;
	}
	dev->last_rx = jiffies;

	if (skb->destructor) {
		skb2 = skb;
		skb = skb_clone(skb, GFP_ATOMIC);
		if (!skb)
			return -1;
	}
	skb->nf_info = info;

	stats = (struct net_device_stats *)dev->priv;
	stats->rx_bytes+= skb->len;
	stats->rx_packets++;

	spin_lock_bh(&dev->queue_lock);
	q = dev->qdisc;
	if (q->enqueue) {
		q->enqueue(skb_get(skb), q);
		if (skb_shared(skb)) {
			skb->destructor = imq_skb_destructor;
			kfree_skb(skb);
			ret = 0;
		}
	}
	if (spin_is_locked(&dev->_xmit_lock))
		netif_schedule(dev);
	else
		while (!netif_queue_stopped(dev) && qdisc_restart1(dev) < 0)
			/* NOTHING */;

	spin_unlock_bh(&dev->queue_lock);

	if (skb2)
		kfree_skb(ret ? skb : skb2);

	return ret;
}

static struct nf_queue_handler nfqh = {
	.name  = "imq",
	.outfn = imq_nf_queue,
};

static unsigned int imq_nf_hook(unsigned int hook, struct sk_buff **pskb,
				const struct net_device *indev,
				const struct net_device *outdev,
				int (*okfn)(struct sk_buff *))
{
	if ((*pskb)->imq_flags & IMQ_F_ENQUEUE)
		return NF_QUEUE;

	return NF_ACCEPT;
}


static int __init imq_init_hooks(void)
{
	int err;

	err = nf_register_queue_handler(PF_INET, &nfqh);
	if (err > 0)
		goto err1;
	if ((err = nf_register_hook(&imq_ingress_ipv4)))
		goto err2;
	if ((err = nf_register_hook(&imq_egress_ipv4)))
		goto err3;
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
	if ((err = nf_register_queue_handler(PF_INET6, &nfqh)))
		goto err4;
	if ((err = nf_register_hook(&imq_ingress_ipv6)))
		goto err5;
	if ((err = nf_register_hook(&imq_egress_ipv6)))
		goto err6;
#endif

	return 0;

#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
err6:
	nf_unregister_hook(&imq_ingress_ipv6);
err5:
	nf_unregister_queue_handler(PF_INET6);
err4:
	nf_unregister_hook(&imq_egress_ipv6);
#endif
err3:
	nf_unregister_hook(&imq_ingress_ipv4);
err2:
	nf_unregister_queue_handler(PF_INET);
err1:
	return err;
}

static void __exit imq_unhook(void)
{
	nf_unregister_hook(&imq_ingress_ipv4);
	nf_unregister_hook(&imq_egress_ipv4);
	nf_unregister_queue_handler(PF_INET);
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
	nf_unregister_hook(&imq_ingress_ipv6);
	nf_unregister_hook(&imq_egress_ipv6);
	nf_unregister_queue_handler(PF_INET6);
#endif
}

static int __init imq_dev_init(struct net_device *dev)
{
	dev->hard_start_xmit    = imq_dev_xmit;
	dev->type               = ARPHRD_VOID;
	dev->mtu                = 16000;
	dev->tx_queue_len       = 11000;
	dev->flags              = IFF_NOARP;
	dev->priv = kmalloc(sizeof(struct net_device_stats), GFP_KERNEL);
	if (dev->priv == NULL)
		return -ENOMEM;
	memset(dev->priv, 0, sizeof(struct net_device_stats));
	dev->get_stats          = imq_get_stats;

	return 0;
}

static void imq_dev_uninit(struct net_device *dev)
{
	kfree(dev->priv);
}

static int __init imq_init_devs(void)
{
	struct net_device *dev;
	int i,j;
	j = numdevs;

	if (!numdevs || numdevs > IMQ_MAX_DEVS) {
		printk(KERN_ERR "IMQ: numdevs has to be betweed 1 and %u\n",
		       IMQ_MAX_DEVS);
		return -EINVAL;
	}

	imq_devs = kmalloc(sizeof(struct net_device) * numdevs, GFP_KERNEL);
	if (!imq_devs)
		return -ENOMEM;
	memset(imq_devs, 0, sizeof(struct net_device) * numdevs);

	/* we start counting at zero */
	numdevs--;

	for (i = 0, dev = imq_devs; i <= numdevs; i++, dev++) {
		SET_MODULE_OWNER(dev);
		strcpy(dev->name, "imq%d");
		dev->init   = imq_dev_init;
		dev->uninit = imq_dev_uninit;

		if (register_netdev(dev) < 0)
			goto err_register;
	}
	printk(KERN_INFO "IMQ starting with %u devices...\n", j);
	return 0;

err_register:
	for (; i; i--)
		unregister_netdev(--dev);
	kfree(imq_devs);
	return -EIO;
}

static void imq_cleanup_devs(void)
{
	int i;
	struct net_device *dev = imq_devs;

	for (i = 0; i <= numdevs; i++)
		unregister_netdev(dev++);

	kfree(imq_devs);
}

static int __init imq_init_module(void)
{
	int err;

	if ((err = imq_init_devs())) {
		printk(KERN_ERR "IMQ: Error trying imq_init_devs()\n");
		return err;
	}
	if ((err = imq_init_hooks())) {
		printk(KERN_ERR "IMQ: Error trying imq_init_hooks()\n");
		imq_cleanup_devs();
		return err;
	}

	printk(KERN_INFO "IMQ driver loaded successfully.\n");

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

static void __exit imq_cleanup_module(void)
{
	imq_unhook();
	imq_cleanup_devs();
	printk(KERN_INFO "IMQ driver unloaded successfully.\n");
}


module_init(imq_init_module);
module_exit(imq_cleanup_module);

module_param(numdevs, int, 0);
MODULE_PARM_DESC(numdevs, "number of IMQ devices (how many imq* devices will be created)");
MODULE_AUTHOR("http://www.linuximq.net");
MODULE_DESCRIPTION("Pseudo-driver for the intermediate queue device. See http://www.linuximq.net/ for more information.");
MODULE_LICENSE("GPL");
