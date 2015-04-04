#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0)
#include <asm/system.h>
#endif

#include "rdm.h"

#define RDM_WIRELESS_ADDR	RALINK_11N_MAC_BASE // wireless control
#define RDM_DEVNAME		"rdm0"

static unsigned int register_control = RDM_SYSCTL_ADDR;
static int rdm_major = 253;

static long rdm_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int rtvalue, baseaddr, offset;
	unsigned int addr=0,count=0;
	
	baseaddr = register_control;
	if (cmd == RT_RDM_CMD_SHOW)
	{
		rtvalue = le32_to_cpu(*(volatile u32 *)(baseaddr + (*(int *)arg)));
		printk("0x%x\n", (int)rtvalue);
	}
	else if (cmd == RT_RDM_CMD_DUMP) 
	{
	        for (count=0; count < RT_RDM_DUMP_RANGE ; count++) {
		    addr = baseaddr + (*(int *)arg) + (count*16);
		    printk("%08X: ", addr);
		    printk("%08X %08X %08X %08X\n", 
			        le32_to_cpu(*(volatile u32 *)(addr)),
				le32_to_cpu(*(volatile u32 *)(addr+4)),
				le32_to_cpu(*(volatile u32 *)(addr+8)),
				le32_to_cpu(*(volatile u32 *)(addr+12)));
		}
	}
	else if (cmd == RT_RDM_CMD_DUMP_FPGA_EMU) 
	{
	        for (count=0; count < RT_RDM_DUMP_RANGE ; count++) {
		    addr = baseaddr + (*(int *)arg) + (count*16);
		    printk("this.cpu_gen.set_reg32('h%08X,'h%08X);\n", addr, le32_to_cpu(*(volatile u32 *)(addr)));
		    printk("this.cpu_gen.set_reg32('h%08X,'h%08X);\n", addr+4, le32_to_cpu(*(volatile u32 *)(addr+4)));
		    printk("this.cpu_gen.set_reg32('h%08X,'h%08X);\n", addr+8, le32_to_cpu(*(volatile u32 *)(addr+8)));
		    printk("this.cpu_gen.set_reg32('h%08X,'h%08X);\n", addr+12, le32_to_cpu(*(volatile u32 *)(addr+12)));
		}
	}
	else if (cmd == RT_RDM_CMD_READ) //also read, but return a value instaead of printing it out
	{
		rtvalue = le32_to_cpu(*(volatile u32 *)(baseaddr + (*(int *)arg)));
		//printk("rtvalue %x\n", rtvalue);
		put_user(rtvalue, (int __user *)arg);
	}
	else if (cmd == RT_RDM_CMD_SET_BASE_SYS)
	{
		register_control = RDM_SYSCTL_ADDR;
		printk("switch register base addr to system register 0x%x\n",RALINK_SYSCTL_BASE);
	}
	else if (cmd == RT_RDM_CMD_SET_BASE_WLAN)
	{
		register_control = RDM_WIRELESS_ADDR;
		printk("switch register base addr to wireless register 0x%08x\n", RDM_WIRELESS_ADDR);
	}
	else if (cmd == RT_RDM_CMD_SHOW_BASE)
	{
		printk("current register base addr is 0x%08x\n", register_control);
	}
	else if (cmd == RT_RDM_CMD_SET_BASE)
	{
		register_control = (*(int *)arg);
		printk("switch register base addr to 0x%08x\n", register_control);
	}
	else if (((cmd & 0xffff) == RT_RDM_CMD_WRITE) || ((cmd & 0xffff) == RT_RDM_CMD_WRITE_SILENT))
	{
		offset = cmd >> 16;
		*(volatile u32 *)(baseaddr + offset) = cpu_to_le32((*(int *)arg));
		if ((cmd & 0xffff) == RT_RDM_CMD_WRITE)
			printk("write offset 0x%x, value 0x%x\n", offset, (unsigned int)(*(int *)arg));
	}else {
		return -EOPNOTSUPP;
	}

	return 0;
}

static int rdm_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

static int rdm_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	return 0;
}

static const struct file_operations rdm_fops = 
{
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= rdm_ioctl,
	.open		= rdm_open,
	.release	= rdm_release,
};

int __init rdm_init(void)
{
	int result = register_chrdev(rdm_major, RDM_DEVNAME, &rdm_fops);
	if (result < 0) {
		printk(KERN_WARNING "ps: can't get major %d\n",rdm_major);
		return result;
	}

	if (rdm_major == 0)
		rdm_major = result; /* dynamic */

	return 0;
}

void __exit rdm_exit(void)
{
	unregister_chrdev(rdm_major, RDM_DEVNAME);
}

module_init(rdm_init);
module_exit(rdm_exit);

module_param (rdm_major, int, 0);

MODULE_LICENSE("GPL");
