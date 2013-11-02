#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/mount.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>
#include "../mtdcore.h"
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/rt2880/rt_mmap.h>
#include "ralink-flash.h"
#ifdef CONFIG_MTD_NOR_RALINK
#include "ralink-flash-map.h"
#endif

#define WINDOW_ADDR		CPHYSADDR(CONFIG_MTD_PHYSMAP_START)
#define WINDOW_SIZE		CONFIG_MTD_PHYSMAP_LEN
#define NUM_FLASH_BANKS		1

#define BUSWIDTH		CONFIG_MTD_PHYSMAP_BUSWIDTH

int ra_check_flash_type(void)
{
	uint8_t Id[10];
	int syscfg=0;
	int chip_mode=0;
#if defined (CONFIG_MTD_NOR_RALINK)
	int boot_from = BOOT_FROM_NOR;
#elif defined (CONFIG_MTD_NAND_RALINK)
	int boot_from = BOOT_FROM_NAND;
#else
	int boot_from = BOOT_FROM_SPI;
#endif
	memset(Id, 0, sizeof(Id));
	strncpy(Id, (char *)RALINK_SYSCTL_BASE, 6);
	syscfg = (*((int *)(RALINK_SYSCTL_BASE + 0x10)));

#if defined(CONFIG_RALINK_RT3052)
	if ((strcmp(Id,"RT3052")==0) || (strcmp(Id, "RT3350")==0)) {
		boot_from = (syscfg >> 16) & 0x3;
		switch(boot_from)
		{
		case 0:
		case 1:
			boot_from = BOOT_FROM_NOR;
			break;
		case 2:
			boot_from = BOOT_FROM_NAND;
			break;
		case 3:
			boot_from = BOOT_FROM_SPI;
			break;
		}
	} else
#elif defined(CONFIG_RALINK_RT3883)
	if (strcmp(Id,"RT3883")==0) {
		boot_from = (syscfg >> 4) & 0x3; 
		switch(boot_from)
		{
		case 0:
		case 1:
			boot_from = BOOT_FROM_NOR;
			break;
		case 2:
		case 3:
			chip_mode = syscfg & 0xF;
			if((chip_mode==0) || (chip_mode==7)) {
				boot_from = BOOT_FROM_SPI;
			}else if(chip_mode==8) {
				boot_from = BOOT_FROM_NAND;
			}else {
				printk("unknow chip_mode=%d\n",chip_mode);
			}
			break;
		}
	} else
#elif defined(CONFIG_RALINK_RT3352)
	if (strcmp(Id,"RT3352")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#elif defined(CONFIG_RALINK_RT5350)
	if (strcmp(Id,"RT5350")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#elif defined(CONFIG_RALINK_RT2880)
	if (strcmp(Id,"RT2880")==0) {
		boot_from = BOOT_FROM_NOR;
	} else
#elif defined(CONFIG_RALINK_RT6855)
	if (strcmp(Id,"RT6855")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#elif defined(CONFIG_RALINK_MT7620)
	if (strcmp(Id,"MT7620")==0) {
		chip_mode = syscfg & 0xF;
		switch(chip_mode)
		{
		case 0:
		case 2:
		case 3:
			boot_from = BOOT_FROM_SPI;
			break;
		case 1:
		case 10:
		case 11:
		case 12:
			boot_from = BOOT_FROM_NAND;
			break;
		}
	} else
#elif defined(CONFIG_RALINK_MT7621)
	if (strcmp(Id,"MT7621")==0) {
		chip_mode = syscfg & 0xF;
		switch(chip_mode)
		{
		case 0:
		case 2:
		case 3:
			boot_from = BOOT_FROM_SPI;
			break;
		case 1:
		case 10:
		case 11:
		case 12:
			boot_from = BOOT_FROM_NAND;
			break;
		}
	} else
#endif
	{
		printk("%s: %s is not supported\n",__FUNCTION__, Id);
	}

	return boot_from;
}

#ifdef CONFIG_MTD_NOR_RALINK
static struct mtd_info *ralink_mtd[NUM_FLASH_BANKS];
static struct map_info ralink_map[] = {
	{
		.name = "Ralink SoC physically mapped flash",
		.bankwidth = BUSWIDTH,
		.size = WINDOW_SIZE,
		.phys = WINDOW_ADDR
	}
};

static int ralink_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}

static int ralink_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	return 0;
}
#endif

#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
typedef struct __image_header {
	uint8_t unused[60];
	uint32_t ih_ksz;
} _ihdr_t;
#endif

static int __init rt2880_mtd_init(void)
{
#ifdef CONFIG_MTD_NOR_RALINK
	int ret = -ENXIO;
	int i, found = 0;

#ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
	_ihdr_t hdr;
	char *ptr = (char *)CKSEG1ADDR(CONFIG_MTD_PHYSMAP_START + MTD_BOOT_PART_SIZE +
			MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE);
	memcpy(&hdr, ptr, sizeof(_ihdr_t));

	if (hdr.ih_ksz != 0) {
		rt2880_partitions[3].size = ntohl(hdr.ih_ksz);
		rt2880_partitions[4].size = IMAGE1_SIZE - (MTD_BOOT_PART_SIZE +
				MTD_CONFIG_PART_SIZE + MTD_FACTORY_PART_SIZE +
				MTD_STORE_PART_SIZE +
				ntohl(hdr.ih_ksz));
	}
#endif

	if(ra_check_flash_type() != BOOT_FROM_NOR) { /* NOR */
		return 0;
	}

	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		printk(KERN_NOTICE "ralink flash device: 0x%x at 0x%x\n",  (unsigned int)ralink_map[i].size, ralink_map[i].phys);

		ralink_map[i].virt = ioremap_nocache(ralink_map[i].phys, ralink_map[i].size);
		if (!ralink_map[i].virt) {
			printk("Failed to ioremap\n");
			return -EIO;
		}
		simple_map_init(&ralink_map[i]);

		ralink_mtd[i] = do_map_probe("cfi_probe", &ralink_map[i]);
		if (ralink_mtd[i]) {
			ralink_mtd[i]->owner = THIS_MODULE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
			ralink_mtd[i]->_lock = ralink_lock;
			ralink_mtd[i]->_unlock = ralink_unlock;
#else
			ralink_mtd[i]->lock = ralink_lock;
			ralink_mtd[i]->unlock = ralink_unlock;
#endif
			++found;
		}
		else
			iounmap(ralink_map[i].virt);
	}
	if (found == NUM_FLASH_BANKS) {
	    if (!found) {
		printk("Error: No Flash device was found\n");
		return -ENXIO;
	    }
		ret = add_mtd_partitions(ralink_mtd[0], rt2880_partitions,
				ARRAY_SIZE(rt2880_partitions));
		if (ret) {
			for (i = 0; i < NUM_FLASH_BANKS; i++)
				iounmap(ralink_map[i].virt);
			return ret;
		}
	}
	else {
		printk("Error: %d flash device was found\n", found);
		return -ENXIO;
	}
#endif
	return 0;
}

static void __exit rt2880_mtd_exit(void)
{
#ifdef CONFIG_MTD_NOR_RALINK
	int i;
	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		if (ralink_mtd[i])
			map_destroy(ralink_mtd[i]);
		if (ralink_map[i].virt) {
			iounmap(ralink_map[i].virt);
			ralink_map[i].virt = NULL;
		}
	}
#endif
}

/*
 * Flash API: ra_mtd_read, ra_mtd_write
 * Arguments:
 *   - num: specific the mtd number
 *   - to/from: the offset to read from or written to
 *   - len: length
 *   - buf: data to be read/written
 * Returns:
 *   - return -errno if failed
 *   - return the number of bytes read/written if successed
 */
int ra_mtd_write_nm(char *name, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;

	mtd = get_mtd_device_nm(name);

	if (IS_ERR(mtd)) {
		ret = (int)mtd;
		goto out;
	}

	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		ret = -E2BIG;
		goto out;
	}

	bak = kzalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		ret = -ENOMEM;
		goto out;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	ret = mtd_read(mtd, 0, mtd->erasesize, &rdlen, bak);
#else
	ret = mtd->read(mtd, 0, mtd->erasesize, &rdlen, bak);
#endif
	if (ret) {
		goto free_out;
	}

	if (rdlen != mtd->erasesize)
		printk("warning: ra_mtd_write_nm: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

	ei.mtd = mtd;
	ei.callback = NULL;
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ei.priv = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	ret = mtd_erase(mtd, &ei);
#else
	ret = mtd->erase(mtd, &ei);
#endif
	if (ret != 0)
		goto free_out;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	ret = mtd_write(mtd, 0, mtd->erasesize, &wrlen, bak);
#else
	ret = mtd->write(mtd, 0, mtd->erasesize, &wrlen, bak);
#endif

	udelay(10); /* add delay after write */

free_out:
	if (mtd)
		put_mtd_device(mtd);

	if (bak)
		kfree(bak);
out:
	return ret;
}

int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen = 0;
	struct mtd_info *mtd;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return (int)mtd;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,0)
	ret = mtd_read(mtd, from, len, &rdlen, buf);
#else
	ret = mtd->read(mtd, from, len, &rdlen, buf);
#endif
	if (rdlen != len)
		printk("warning: ra_mtd_read_nm: rdlen is not equal to len\n");

	put_mtd_device(mtd);
	return ret;
}

module_init(rt2880_mtd_init);
module_exit(rt2880_mtd_exit);

EXPORT_SYMBOL(ra_mtd_write_nm);
EXPORT_SYMBOL(ra_mtd_read_nm);

MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_DESCRIPTION("Ralink APSoC Flash Map");
MODULE_LICENSE("GPL");
