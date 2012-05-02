/*
 * passthrough 
 * This module is used to pass through the specific packets
 */

#include <linux/netdevice.h>
#include <linux/proc_fs.h>

#define PT_ROOT		"pthrough"

extern int pppoe_pt_enable;
extern int ipv6_pt_enable;
extern int pppoe_pthrough(struct sk_buff *skb);
extern int ipv6_pthrough(struct sk_buff *skb);

/* return 1, if we want to handle this packet, or
 * return 0, let other ones do this	 */
int private_pthrough(struct sk_buff *skb)
{
	int ret = 0;

	if (pppoe_pt_enable) {
		ret = pppoe_pthrough(skb);
	}
	if (!ret && ipv6_pt_enable) {
		ret = ipv6_pthrough(skb);
	}
	
	return ret;
}

static struct proc_dir_entry * root = NULL;
static struct proc_dir_entry * pppoe = NULL;
static struct proc_dir_entry * ipv6 = NULL;

extern int proc_pppoe_read(char *page, char **start, off_t off, int count, int *eof, void *data);
extern int proc_pppoe_write(struct file *file, const char * buffer, unsigned long count, void *data);
extern int proc_ipv6_read(char *page, char **start, off_t off, int count, int *eof, void *data);
extern int proc_ipv6_write(struct file *file, const char * buffer, unsigned long count, void *data);

void pthrough_remove_proc_entry(void)
{
	if (pppoe) {
		remove_proc_entry("pppoe", root);
		pppoe = NULL;
	}
	if (ipv6) {
		remove_proc_entry("ipv6", root);
		ipv6 = NULL;
	}
	if (root) {
		remove_proc_entry(PT_ROOT, NULL);
		root = NULL;
	}
}

int pthrough_create_proc_entry(void)
{
	/* create directory */
	root = proc_mkdir(PT_ROOT, NULL);
	if (root == NULL) {
		printk("proc_mkdir return NULL!\n");
		goto pt_out;
	}

	/* create entries */
	pppoe = create_proc_entry("pppoe", 0644, root);
	if (pppoe == NULL) {
		printk("create_proc_entry (pppoe) return NULL!\n");
		goto pt_out;
	}
	pppoe->read_proc = proc_pppoe_read;
	pppoe->write_proc = proc_pppoe_write;
	
	ipv6 = create_proc_entry("ipv6", 0644, root);
	if (ipv6 == NULL) {
		printk("create_proc_entry (ipv6) return NULL!\n");
		goto pt_out;
	}
	ipv6->read_proc = proc_ipv6_read;
	ipv6->write_proc = proc_ipv6_write;
	
	return 0;
	
pt_out:
	pthrough_remove_proc_entry();
	printk("Unable to create %s !!\n", PT_ROOT);
	return -1;
}
