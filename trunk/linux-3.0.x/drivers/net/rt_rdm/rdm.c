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
#include <linux/wireless.h>
#include "rdm.h"

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif

#define RDM_WIRELESS_ADDR	RALINK_11N_MAC_BASE // wireless control
#define RDM_DEVNAME		"rdm0"
static int register_control = RDM_WIRELESS_ADDR;
#ifdef  CONFIG_DEVFS_FS
static devfs_handle_t devfs_handle;
#endif
int rdm_major = 253;

int rdm_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int rdm_release(struct inode *inode, struct file *filp)
{
	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long rdm_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
#else
int rdm_ioctl (struct inode *inode, struct file *filp,
                     unsigned int cmd, unsigned long *arg)
#endif
{
	unsigned int rtvalue, baseaddr, offset;
	unsigned int addr=0,count=0;
	
	//printk("rdm_ioctl\n");
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

struct file_operations rdm_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    unlocked_ioctl:      rdm_ioctl,
#else
    ioctl:      rdm_ioctl,
#endif
    open:       rdm_open,
    release:    rdm_release,
};

static int rdm_init(void)

{
#ifdef  CONFIG_DEVFS_FS
	if(devfs_register_chrdev(rdm_major, RDM_DEVNAME , &rdm_fops)) {
		printk(KERN_WARNING " ps: can't create device node - ps\n");
		return -EIO;
	}

	devfs_handle = devfs_register(NULL, RDM_DEVNAME, DEVFS_FL_DEFAULT, rdm_major, 0, 
				S_IFCHR | S_IRUGO | S_IWUGO, &rdm_fops, NULL);
#else
	int result=0;
	result = register_chrdev(rdm_major, RDM_DEVNAME, &rdm_fops);
	if (result < 0) {
	printk(KERN_WARNING "ps: can't get major %d\n",rdm_major);
	return result;
	}

	if (rdm_major == 0) {
		rdm_major = result; /* dynamic */
	}
#endif

	printk("rdm_major = %d\n", rdm_major);
	return 0;
}



static void rdm_exit(void)
{
	printk("rdm_exit\n");

#ifdef  CONFIG_DEVFS_FS
	devfs_unregister_chrdev(rdm_major, RDM_DEVNAME);
	devfs_unregister(devfs_handle);
#else
	unregister_chrdev(rdm_major, RDM_DEVNAME);
#endif

}

module_init(rdm_init);
module_exit(rdm_exit);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (rdm_major, "i");
#else
module_param (rdm_major, int, 0);
#endif

MODULE_LICENSE("GPL");
