#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>   
#include <linux/fs.h>       
#include <linux/errno.h>    
#include <linux/types.h>    
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    
#include <asm/system.h>     
#include <linux/wireless.h>
#include <asm/uaccess.h>

#include "flash.h"
#include "flash_ioctl.h"

#ifdef  CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
static	devfs_handle_t devfs_handle;
#endif

static int s_flash_major =  200;
static struct class *s_flash_class = NULL;
static struct device *s_flash_device = NULL;


int flash_ioctl (struct inode *inode, struct file *filp,
                     unsigned int cmd, unsigned long arg)
{
    struct flash_opt *opt=(struct flash_opt *)arg;
    unsigned char *buf;
    unsigned int tmp;
    unsigned int start_sect=0,end_sect=0;

    buf=kmalloc(FLASH_MAX_RW_SIZE, GFP_KERNEL);

    switch(cmd) 
    {
    case FLASH_IOCTL_READ:
	if(FlashRead( (unsigned int *)buf, opt->src, opt->bytes) < 0) {
	    opt->result=OUT_OF_SCOPE;
	}
	copy_to_user((char *)opt->dest,buf, opt->bytes);
	break;
    case FLASH_IOCTL_WRITE:
	copy_from_user( buf, (char *)opt->src, opt->bytes);
	if(FlashWrite((unsigned short *)buf, (unsigned short *)opt->dest, opt->bytes)<0){
	    opt->result = OUT_OF_SCOPE;
	}
	break;
    case FLASH_IOCTL_ERASE:
	if(FlashGetSector(opt->start_addr, &start_sect) && FlashGetSector(opt->end_addr, &end_sect)){
		printk("Erase Sector From %d To %d \n",start_sect, end_sect);
		if(FlashErase(start_sect, end_sect)<0) {
			opt->result = OUT_OF_SCOPE;
		}
	}
	break;
    default:
	break;
    }

    kfree(buf);
    return 0;
}

struct file_operations flash_fops = {
    ioctl:      flash_ioctl,
};


static int flash_init(void)
{

#ifdef  CONFIG_DEVFS_FS
    if(devfs_register_chrdev(s_flash_major, FLASH_DEVNAME , &flash_fops)) {
	printk(KERN_WARNING " flash: can't create device node - %s\n",FLASH_DEVNAME);
	return -EIO;
    }

    devfs_handle = devfs_register(NULL, FLASH_DEVNAME, DEVFS_FL_DEFAULT, s_flash_major, 0,
	    S_IFCHR | S_IRUGO | S_IWUGO, &flash_fops, NULL);
#else
    int result=0, ret = 0;
    struct class *tmp_class;
    struct device *tmp_device;

    result = register_chrdev(s_flash_major, FLASH_DEVNAME, &flash_fops);
    if (result < 0) {
	printk(KERN_WARNING "flash: can't get major %d\n",s_flash_major);
        return result;
    }

    if (s_flash_major == 0) {
	s_flash_major = result; /* dynamic */
    }

	tmp_class = class_create(THIS_MODULE, FLASH_DEVNAME);
	if (IS_ERR(tmp_class)) {
		ret = PTR_ERR(tmp_class);
		goto err_class_create;
	}
	s_flash_class = tmp_class;
	tmp_device = device_create(s_flash_class, NULL, MKDEV(s_flash_major, 0), "%s", FLASH_DEVNAME);
	if (IS_ERR(tmp_device)) {
		ret = PTR_ERR(tmp_device);
		goto err_device_create;
	}
	s_flash_device = tmp_device;
#endif

    return 0;

err_device_create:

	class_destroy(s_flash_class);
	s_flash_class = NULL;

err_class_create:

	unregister_chrdev(s_flash_major, FLASH_DEVNAME);

	return ret;
}



static void flash_exit(void)
{
    printk("flash_exit\n");

#ifdef  CONFIG_DEVFS_FS
    devfs_unregister_chrdev(s_flash_major, FLASH_DEVNAME);
    devfs_unregister(devfs_handle);
#else

	if (s_flash_device) {
		device_destroy(s_flash_class, MKDEV(s_flash_major, 0));
		s_flash_device = NULL;
	}
	if (s_flash_class) {
		class_destroy(s_flash_class);
		s_flash_class = NULL;
	}

    unregister_chrdev(s_flash_major, FLASH_DEVNAME);
#endif

}

module_init(flash_init);
module_exit(flash_exit);
MODULE_LICENSE("GPL");
