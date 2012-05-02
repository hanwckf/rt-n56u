/*
 * IPv6 passthrough 
 * This module is used to pass through the IPv6 packets
 */

#include <linux/netdevice.h>
#include <linux/proc_fs.h>

//#define DEBUG
#define ETH_TYPE_IPV6	0x86dd

extern struct net_device *dev_get_by_name(const char *name);
extern int dev_set_promiscuity(struct net_device *dev, int inc);
extern int dev_queue_xmit(struct sk_buff *skb);

int ipv6_pt_enable = 0;

char ipv6_pt_landev[IFNAMSIZ];
char ipv6_pt_wandev[IFNAMSIZ];

/* return 1, if we want to handle this packet, or
 * return 0, let other ones do this	 */
int ipv6_pthrough(struct sk_buff *skb)
{
	unsigned short proto;
	struct net_device *dev;

	/* check if ipv6 pass through enabled or not
	 * if not set yet, just return and do nothing */
	if (!ipv6_pt_enable) 
	    return 0;
	
	// check and forward packets
	proto = 0x100*(skb->mac.raw[12]) + skb->mac.raw[13];

	if (proto == ETH_TYPE_IPV6) {
		if (strcmp(skb->dev->name, ipv6_pt_landev) == 0) {
#ifdef DEBUG
			printk("ipv6_pass: OutGoing packet (%s)\n", skb->dev->name);
#endif
			dev = dev_get_by_name(ipv6_pt_wandev);
			if (!dev)
				return 0;
			else {
				skb->dev=dev;
				dev_put(skb->dev);
			}
			skb_push(skb, ETH_HLEN);
			dev_queue_xmit(skb);
			return 1;
		}
		if (strcmp(skb->dev->name, ipv6_pt_wandev) == 0) {
#ifdef DEBUG
			printk("ipv6_pass: PPPoE Incoming packet (%s)\n", skb->dev->name);
#endif
			dev = dev_get_by_name(ipv6_pt_landev);
			if (!dev)
				return 0;
			else {
				skb->dev=dev;
				dev_put(skb->dev);
			}
			skb_push(skb, ETH_HLEN);
			dev_queue_xmit(skb);
			return 1;
		}
	}

	return 0;
}

int proc_ipv6_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret=0;
	if (ipv6_pt_enable) {
		ret = sprintf(page, "%s,%s\n", ipv6_pt_landev, ipv6_pt_wandev);
	} else {
		ret = sprintf(page, "null,null\n");
	}

	return ret;
}

#define isCHAR(x) ((x >= 'a') && (x <= 'z')) ? 1:((x >= '0') && (x <= '9')) ? 1:((x >= 'A') && (x <= 'Z')) ? 1:(x == '.') ? 1:0
int proc_ipv6_write(struct file *file, const char * buffer, unsigned long count, void *data)
{
	char *pt;
	int cur_count=0;
	struct net_device *dev;

	if (ipv6_pt_enable) {
		ipv6_pt_enable = 0;
		if ((dev = dev_get_by_name(ipv6_pt_landev))) {
			dev_set_promiscuity(dev, -1);
			dev_put(dev);
		}
		if ((dev = dev_get_by_name(ipv6_pt_wandev))) {
			dev_set_promiscuity(dev, -1);
			dev_put(dev);
		}
	}

	/* we expect that buffer contain format of "landev_name,wandev_name" */
	memset(ipv6_pt_landev, 0x0, sizeof (ipv6_pt_landev));
	for (pt=ipv6_pt_landev; (*buffer && (*buffer != ',') && (*buffer != '\n')); buffer++) {
		if ((*buffer != ' ') && isCHAR(*buffer)) {
			*pt = *buffer;
			pt++;
		}
	    cur_count++;
	    if (cur_count >= count)
		break;
	}
	
	if (!(*buffer) || (cur_count >= count))	{
	    printk("ipv6_pass: need <lan>,<wan>; device name format\n");
	    goto ppw_failed;
	}
	
	memset(ipv6_pt_wandev, 0x0, sizeof (ipv6_pt_wandev));
	for (pt=ipv6_pt_wandev, buffer++; (*buffer && (*buffer != ';') && (*buffer != '\n')); buffer++) {
		if ((*buffer != ' ') && isCHAR(*buffer)) {
			*pt = *buffer;
			pt++;
		}
	    cur_count++;
	    if (cur_count >= count)
		break;
	}
	
	if (!(dev = dev_get_by_name(ipv6_pt_landev))) {
#ifdef DEBUG
	    printk("ipv6_pass: no lan device %s)\n",ipv6_pt_landev); 
#endif
	    goto ppw_failed;
	}
	else 
	{
	    dev_set_promiscuity(dev, 1);
	    dev_put(dev);
	}

	if (!(dev = dev_get_by_name(ipv6_pt_wandev))) {
#ifdef DEBUG
	    printk("ipv6_pass: no wan device %s)\n",ipv6_pt_wandev);
#endif
	    goto ppw_failed;
	}
	else 
	{
	    dev_set_promiscuity(dev, 1);
	    dev_put(dev);
	}
	
	ipv6_pt_enable = 1;
	printk("ipv6 pass through (%s<->%s)\n",ipv6_pt_landev, ipv6_pt_wandev);
	return count;
	
ppw_failed:
	ipv6_pt_enable = 0;
	memset(ipv6_pt_landev, 0x0, sizeof (ipv6_pt_landev));
	memset(ipv6_pt_wandev, 0x0, sizeof (ipv6_pt_wandev));
	printk("ipv6_pass: Disabled\n");
	return count;
}
