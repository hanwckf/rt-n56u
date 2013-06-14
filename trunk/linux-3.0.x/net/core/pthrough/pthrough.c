/*
 * passthrough 
 * This module is used to pass through the specific packets
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>

#define PT_ROOT		"pthrough"

static struct proc_dir_entry *pt_de_root = NULL;
static struct proc_dir_entry *pt_de_pppoe = NULL;

extern int proc_pppoe_read(char *page, char **start, off_t offset, int count, int *eof, void *data);
extern int proc_pppoe_write(struct file *file, const char __user *buffer, unsigned long count, void *data);

void pthrough_remove_proc_entry(struct net *net)
{
	if (pt_de_pppoe) {
		remove_proc_entry("pppoe", pt_de_root);
		pt_de_pppoe = NULL;
	}
	if (pt_de_root) {
		remove_proc_entry(PT_ROOT, net->proc_net);
		pt_de_root = NULL;
	}
}

int pthrough_create_proc_entry(struct net *net)
{
	/* create directory */
	pt_de_root = proc_mkdir(PT_ROOT, net->proc_net);
	if (!pt_de_root)
		goto pt_out;

	/* create entries */
	pt_de_pppoe = create_proc_entry("pppoe", 0644, pt_de_root);
	if (!pt_de_pppoe)
		goto pt_out;
	
	pt_de_pppoe->read_proc = proc_pppoe_read;
	pt_de_pppoe->write_proc = proc_pppoe_write;
	
	return 0;
	
pt_out:
	pthrough_remove_proc_entry(net);
	return -ENOMEM;
}

