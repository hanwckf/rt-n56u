/*
 * PPPoE passthrough 
 * This module is used to pass through the PPPoE packets
 */

#include <linux/netdevice.h>
#include <linux/proc_fs.h>

//#define DEBUG
#define ETH_TYPE_PPPOE_DISCOVERY	0x8863
#define ETH_TYPE_PPPOE_SESSION		0x8864
#define PTABLE_SIZE 			16

extern struct net_device *dev_get_by_name(const char *name);
extern int dev_set_promiscuity(struct net_device *dev, int inc);
extern int dev_queue_xmit(struct sk_buff *skb);

int pppoe_pt_enable = 0;

char pppoe_pt_landev[IFNAMSIZ];
char pppoe_pt_wandev[IFNAMSIZ];

static int pthrough_idx = 0;
static unsigned char pthrough_table[PTABLE_SIZE][ETH_ALEN];

/* return 1, if we want to handle this packet, or
 * return 0, let other ones do this	 */
int pppoe_pthrough(struct sk_buff *skb)
{
	unsigned short proto;
	unsigned char *smac;
	unsigned char *dmac;
	struct net_device *dev;
	int i;

	/* check if pppoe pass through enabled or not
	 * if not set yet, just return and do nothing */
	
	// check and forward packets
	proto = 0x100*(skb->mac.raw[12]) + skb->mac.raw[13];
	dmac = skb->mac.raw;
	smac = dmac + 6;

	if ((proto == ETH_TYPE_PPPOE_SESSION) || (proto == ETH_TYPE_PPPOE_DISCOVERY)) {
		if (strcmp(skb->dev->name, pppoe_pt_landev) == 0) {
#ifdef DEBUG
			printk("pppoe_pass: PPPoE OutGoing packet (%s)\n", skb->dev->name);
#endif
			for (i=0; i<pthrough_idx; i++) {
				if (!memcmp(pthrough_table[i], smac, ETH_ALEN)) {
					break;
				}
			}
			if (i == pthrough_idx) {
				memcpy(pthrough_table[i], smac, ETH_ALEN);
				pthrough_idx++;
				if (pthrough_idx >= PTABLE_SIZE) {
					printk("pppoe_pass: pthrough_table full!! (%d)\n", pthrough_idx);
					pthrough_idx--;
				}
			}

			dev = dev_get_by_name(pppoe_pt_wandev);
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
		if (strcmp(skb->dev->name, pppoe_pt_wandev) == 0) {
#ifdef DEBUG
			printk("pppoe_pass: PPPoE Incoming packet (%s)\n", skb->dev->name);
#endif
			for (i=0; i<pthrough_idx; i++) {
				if (!memcmp(pthrough_table[i], dmac, ETH_ALEN)) {
					dev = dev_get_by_name(pppoe_pt_landev);
					if (!dev)
						return 0;
					else {
						skb->dev = dev;
						dev_put(skb->dev);
					}
					skb_push(skb, ETH_HLEN);
					dev_queue_xmit(skb);
					return 1;
				}
			}
		}
	}

	return 0;
}

int proc_pppoe_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int ret=0;
	if (pppoe_pt_enable) {
		ret = sprintf(page, "%s,%s\n", pppoe_pt_landev, pppoe_pt_wandev);
	} else {
		ret = sprintf(page, "null,null\n");
	}

	return ret;
}

#define isCHAR(x) ((x >= 'a') && (x <= 'z')) ? 1:((x >= '0') && (x <= '9')) ? 1:((x >= 'A') && (x <= 'Z')) ? 1:(x == '.') ? 1:0
int proc_pppoe_write(struct file *file, const char * buffer, unsigned long count, void *data)
{
	char *pt;
	int cur_count=0;
	struct net_device *dev;

	if (pppoe_pt_enable) {
		pppoe_pt_enable = 0;
		if ((dev = dev_get_by_name(pppoe_pt_landev))) {
			dev_set_promiscuity(dev, -1);
			dev_put(dev);
		}
		if ((dev = dev_get_by_name(pppoe_pt_wandev))) {
			dev_set_promiscuity(dev, -1);
			dev_put(dev);
		}
	}

	/* we expect that buffer contain format of "landev_name,wandev_name" */
	memset(pppoe_pt_landev, 0x0, sizeof (pppoe_pt_landev));
	for (pt=pppoe_pt_landev; (*buffer && (*buffer != ',') && (*buffer != '\n')) ; buffer++) {
		if ((*buffer != ' ') && isCHAR(*buffer)) {
			*pt = *buffer;
			pt++;
		}
	    cur_count++;
	    if (cur_count >= count)
		break;
	}
	
	if (!(*buffer) || (cur_count >= count))	{
	    printk("pppoe_pass: need <lan>,<wan>; device name format\n"); 
	    goto ppw_failed;
	}
	
	memset(pppoe_pt_wandev, 0x0, sizeof (pppoe_pt_wandev));
	for (pt=pppoe_pt_wandev, buffer++; (*buffer && (*buffer != ';') && (*buffer != '\n')); buffer++) {
		if ((*buffer != ' ') && isCHAR(*buffer)) {
			*pt = *buffer;
			pt++;
		}
	    cur_count++;
	    if (cur_count >= count)
		break;
	}
	
	if (!(dev = dev_get_by_name(pppoe_pt_landev))) {
#ifdef DEBUG
	    printk("pppoe_pass: no lan device %s)\n",pppoe_pt_landev); 
#endif
	    goto ppw_failed;
	}
	else 
	{
	    dev_set_promiscuity(dev, 1);
	    dev_put(dev);
	}

	if (!(dev = dev_get_by_name(pppoe_pt_wandev))) {
#ifdef DEBUG
	    printk("pppoe_pass: no wan device %s)\n",pppoe_pt_wandev); 
#endif
	    goto ppw_failed;
	}
	else 
	{
	    dev_set_promiscuity(dev, 1);
	    dev_put(dev);
	}
	
	pppoe_pt_enable = 1;
	printk("pppoe pass through (%s<->%s)\n",pppoe_pt_landev, pppoe_pt_wandev);
	return count;
	
ppw_failed:
	pppoe_pt_enable = 0;
	memset(pppoe_pt_landev, 0x0, sizeof (pppoe_pt_landev));
	memset(pppoe_pt_wandev, 0x0, sizeof (pppoe_pt_wandev));
	printk("pppoe_pass: Disabled.\n");
	return count;
}
