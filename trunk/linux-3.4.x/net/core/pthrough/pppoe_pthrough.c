/*
 * PPPoE passthrough 
 * This module is used to pass through the PPPoE packets
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/proc_fs.h>

//#define PTHROUGH_DEBUG
#define PTHROUGH_LOG		"pppoe-pthrough:"
#define PPP_TABLE_SIZE		16

int pppoe_pt_enable = 0;

static char pppoe_pt_landev[IFNAMSIZ];
static char pppoe_pt_wandev[IFNAMSIZ];

static struct net_device *pppoe_dev_lan = NULL;
static struct net_device *pppoe_dev_wan = NULL;

static int pthrough_idx = 0;
static unsigned char pthrough_table[PPP_TABLE_SIZE][ETH_ALEN];

static inline
void passthrough_skb(struct sk_buff *skb, struct net_device *dev_pass)
{
	skb->dev = dev_pass;
	skb_push(skb, ETH_HLEN);
	dev_queue_xmit(skb);
}

/* return 1, if we want to handle this packet, or
 * return 0, let other ones do this */
int pppoe_pthrough(struct sk_buff *skb)
{
	int i;
	struct net_device *dev_lan, *dev_wan;
	struct ethhdr *eth;

	if (skb->protocol == htons(ETH_P_PPP_SES) || skb->protocol == htons(ETH_P_PPP_DISC)) {
		dev_lan = pppoe_dev_lan;
		dev_wan = pppoe_dev_wan;
		if (!dev_lan || !dev_wan || !skb->dev)
			return 0;
		eth = (struct ethhdr *)skb_mac_header(skb);
		if (skb->dev == dev_lan) {
#ifdef PTHROUGH_DEBUG
			printk("%s PPPoE outgoing packet (%s), "
				"DMAC=%02X:%02X:%02X:%02X:%02X:%02X, "
				"SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n", PTHROUGH_LOG, skb->dev->name, 
				eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
				eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
				eth->h_source[0], eth->h_source[1], eth->h_source[2],
				eth->h_source[3], eth->h_source[4], eth->h_source[5]);
#endif
			if (!compare_ether_addr(dev_wan->dev_addr, eth->h_source))
				return 0;
			
			for (i=0; i<pthrough_idx; i++) {
				if (!compare_ether_addr(pthrough_table[i], eth->h_source))
					break;
			}
			if (i == pthrough_idx) {
				if (pthrough_idx < PPP_TABLE_SIZE) {
					memcpy(pthrough_table[i], eth->h_source, ETH_ALEN);
					pthrough_idx++;
				}
#ifdef PTHROUGH_DEBUG
				else
					printk("%s PPPoE table full!! (%d)\n", PTHROUGH_LOG, pthrough_idx);
#endif
			}
			
			passthrough_skb(skb, dev_wan);
			return 1;
		}
		else
		if (skb->dev == dev_wan) {
#ifdef PTHROUGH_DEBUG
			printk("%s PPPoE incoming packet (%s), "
				"DMAC=%02X:%02X:%02X:%02X:%02X:%02X, "
				"SMAC=%02X:%02X:%02X:%02X:%02X:%02X\n", PTHROUGH_LOG, skb->dev->name,
				eth->h_dest[0], eth->h_dest[1], eth->h_dest[2],
				eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
				eth->h_source[0], eth->h_source[1], eth->h_source[2],
				eth->h_source[3], eth->h_source[4], eth->h_source[5]);
#endif
			for (i=0; i<pthrough_idx; i++) {
				if (!compare_ether_addr(pthrough_table[i], eth->h_dest)) {
					passthrough_skb(skb, dev_lan);
					return 1;
				}
			}
		}
	}

	return 0;
}

int proc_pppoe_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
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
int proc_pppoe_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	char *pt;
	int cur_count=0, pt_enable_last;
	struct net_device *dev_lan, *dev_wan;

	pt_enable_last = pppoe_pt_enable;

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
	
	if (!(*buffer) || (cur_count >= count)){
		printk("%s use <lan>,<wan> device name format!\n", PTHROUGH_LOG);
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
	
	if (!(dev_lan = dev_get_by_name(&init_net, pppoe_pt_landev))) {
#ifdef PTHROUGH_DEBUG
		printk("%s no %s device (%s)\n", PTHROUGH_LOG, "lan", pppoe_pt_landev);
#endif
		goto ppw_failed;
	}

	if (!(dev_wan = dev_get_by_name(&init_net, pppoe_pt_wandev))) {
#ifdef PTHROUGH_DEBUG
		printk("%s no %s device (%s)\n", PTHROUGH_LOG, "wan", pppoe_pt_wandev);
#endif
		dev_put(dev_lan);
		goto ppw_failed;
	}
	
	rtnl_lock();
	if (pppoe_dev_lan) {
		if (pppoe_dev_lan != dev_lan)
			dev_set_promiscuity(pppoe_dev_lan, -1);
		dev_put(pppoe_dev_lan);
	}
	if (pppoe_dev_wan) {
		if (pppoe_dev_wan != dev_wan)
			dev_set_promiscuity(pppoe_dev_wan, -1);
		dev_put(pppoe_dev_wan);
	}
	if (pppoe_dev_lan != dev_lan) {
		dev_set_promiscuity(dev_lan, 1);
		pppoe_dev_lan = dev_lan;
	}
	if (pppoe_dev_wan != dev_wan) {
		dev_set_promiscuity(dev_wan, 1);
		pppoe_dev_wan = dev_wan;
	}
	pppoe_pt_enable = 1;
	rtnl_unlock();
	
	if (pt_enable_last != pppoe_pt_enable)
		printk("%s [%s]<->[%s]\n", PTHROUGH_LOG, pppoe_pt_landev, pppoe_pt_wandev);
	return count;
	
ppw_failed:
	
	rtnl_lock();
	pppoe_pt_enable = 0;
	if (pppoe_dev_lan) {
		dev_set_promiscuity(pppoe_dev_lan, -1);
		dev_put(pppoe_dev_lan);
		pppoe_dev_lan = NULL;
	}
	if (pppoe_dev_wan) {
		dev_set_promiscuity(pppoe_dev_wan, -1);
		dev_put(pppoe_dev_wan);
		pppoe_dev_wan = NULL;
	}
	pthrough_idx = 0;
	memset(pthrough_table, 0x0, sizeof(pthrough_table));
	rtnl_unlock();
	
	memset(pppoe_pt_landev, 0x0, sizeof(pppoe_pt_landev));
	memset(pppoe_pt_wandev, 0x0, sizeof(pppoe_pt_wandev));
	
	if (pt_enable_last != pppoe_pt_enable)
		printk("%s disabled\n", PTHROUGH_LOG);
	return count;
}
