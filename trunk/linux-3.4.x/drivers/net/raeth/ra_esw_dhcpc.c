#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sched.h>

#include "ra_esw_base.h"

#define UDHCPC_PID_FILE		"/var/run/udhcpc.pid"

extern int send_sigusr_dhcpc;

static void send_kill_sig(void)
{
	struct file *fp;
	char pid[8];
	struct task_struct *p = NULL;

	/* read udhcpc pid from file, and send signal USR1 to get a new IP */
	fp = filp_open(UDHCPC_PID_FILE, O_RDONLY, 0);
	if (IS_ERR(fp))
		return;

	if (fp->f_op && fp->f_op->read && fp->f_op->read(fp, pid, 8, &fp->f_pos) > 0) {
		p = pid_task(find_get_pid(simple_strtoul(pid, NULL, 10)), PIDTYPE_PID);
		if (p) {
			printk("ESW: call DHCP client for renew address.\n");
			send_sig(SIGUSR1, p, 0);
		}
	}

	filp_close(fp, NULL);
}

static void esw_link_status_changed(u32 port_id, int port_link)
{
	u32 port_no_r;
	char *port_desc, *port_state_desc;

	/* if set 9 - Disable touch dhcp */
	if (send_sigusr_dhcpc == 9)
		return;

	/* reverse direction ports for compat with classic wive tree */
	port_no_r = 4 - port_id;

	if (port_no_r == send_sigusr_dhcpc)
		port_desc = "WAN ";
	else
		port_desc = "";

	if (port_link) {
		port_state_desc = "Up";
		if (port_no_r == (u32)send_sigusr_dhcpc)
			send_kill_sig();
	} else {
		port_state_desc = "Down";
	}

	printk("ESW: %sLink Status Changed - Port%d Link %s\n", port_desc, port_no_r, port_state_desc);
}

void
esw_dhcpc_init(void)
{
	esw_link_status_hook = esw_link_status_changed;
}

