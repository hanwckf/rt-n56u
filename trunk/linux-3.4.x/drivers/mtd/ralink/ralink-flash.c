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
#if defined (CONFIG_MTD_NOR_RALINK)
#include "ralink-flash-map.h"
#endif

int ra_check_flash_type(void)
{
	uint8_t Id[10];
	int syscfg=0;
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

#if defined (CONFIG_RALINK_RT3052)
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
#elif defined (CONFIG_RALINK_RT3883)
	if (strcmp(Id,"RT3883")==0) {
		int chip_mode = syscfg & 0xF;
		boot_from = (syscfg >> 4) & 0x3;
		switch(boot_from)
		{
		case 0:
		case 1:
			boot_from = BOOT_FROM_NOR;
			break;
		case 2:
		case 3:
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
#elif defined (CONFIG_RALINK_RT3352)
	if (strcmp(Id,"RT3352")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#elif defined (CONFIG_RALINK_RT5350)
	if (strcmp(Id,"RT5350")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#elif defined (CONFIG_RALINK_MT7620)
	if (strcmp(Id,"MT7620")==0) {
		int chip_mode = syscfg & 0xF;
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
#elif defined (CONFIG_RALINK_MT7621)
	if (strcmp(Id,"MT7621")==0) {
		int chip_mode = syscfg & 0xF;
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
#elif defined (CONFIG_RALINK_MT7628)
	if(strcmp(Id,"MT7628")==0) {
		boot_from = BOOT_FROM_SPI;
	} else
#endif
	{
		printk("%s: %s is not supported\n",__FUNCTION__, Id);
	}

	return boot_from;
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
	int ret, res;
	loff_t ofs, ofs_align, bad_shift;
	size_t cnt, i_len, r_len, w_len;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak, *ptr;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return (int)mtd;

	ret = 0;

	if ((to + len) > mtd->size) {
		ret = -E2BIG;
		printk("%s: out of mtd size (%lld)!\n", __FUNCTION__, mtd->size);
		goto out;
	}

	bak = kzalloc(mtd->erasesize, GFP_KERNEL);
	if (!bak) {
		ret = -ENOMEM;
		goto out;
	}

	ptr = (u_char *)buf;
	ofs = to;
	cnt = len;

	bad_shift = 0;

	while (cnt > 0) {
		ofs_align = ofs & ~(mtd->erasesize - 1);	/* aligned to erase boundary */
		i_len = mtd->erasesize - (ofs - ofs_align);
		if (cnt < i_len)
			i_len = cnt;
		
		ei.mtd = mtd;
		ei.callback = NULL;
		ei.addr = ofs_align + bad_shift;
		ei.len = mtd->erasesize;
		ei.priv = 0;
		
		/* check bad block */
		if (mtd->type == MTD_NANDFLASH || mtd->type == MTD_MLCNANDFLASH) {
			if ((ei.addr + mtd->erasesize) > mtd->size) {
				ret = -E2BIG;
				goto free_out;
			}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
			res = mtd_block_isbad(mtd, ei.addr);
#else
			res = 0;
			if (mtd->block_isbad)
				res = mtd->block_isbad(mtd, ei.addr);
#endif
			if (res < 0) {
				ret = -EIO;
				goto free_out;
			}
			if (res > 0) {
				bad_shift += mtd->erasesize;
				continue;
			}
		}
		
		/* backup */
		r_len = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
		res = mtd_read(mtd, ei.addr, mtd->erasesize, &r_len, bak);
#else
		res = mtd->read(mtd, ei.addr, mtd->erasesize, &r_len, bak);
#endif
		if (res || mtd->erasesize != r_len) {
			ret = -EIO;
			printk("%s: read from 0x%llx failed!\n", __FUNCTION__, ei.addr);
			goto free_out;
		}
		
		/* erase */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
		res = mtd_erase(mtd, &ei);
#else
		res = mtd->erase(mtd, &ei);
#endif
		if (res) {
			ret = -EIO;
			printk("%s: erase on 0x%llx failed!\n", __FUNCTION__, ei.addr);
			goto free_out;
		}
		
		/* write */
		w_len = 0;
		memcpy(bak + (ofs - ofs_align), ptr, i_len);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
		res = mtd_write(mtd, ei.addr, mtd->erasesize, &w_len, bak);
#else
		res = mtd->write(mtd, ei.addr, mtd->erasesize, &w_len, bak);
#endif
		if (res || mtd->erasesize != w_len) {
			ret = -EIO;
			printk("%s: write to 0x%llx failed!\n", __FUNCTION__, ei.addr);
			goto free_out;
		}
		
		ptr += i_len;
		ofs += i_len;
		cnt -= i_len;
		
		/* add delay after write */
		udelay(10);
	}

free_out:
	kfree(bak);
out:
	put_mtd_device(mtd);
	return ret;
}
EXPORT_SYMBOL(ra_mtd_write_nm);

int ra_mtd_read_nm(char *name, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t r_len = 0;
	struct mtd_info *mtd;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return (int)mtd;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)
	ret = mtd_read(mtd, from, len, &r_len, buf);
#else
	ret = mtd->read(mtd, from, len, &r_len, buf);
#endif
	if (ret) {
		printk("%s: read from 0x%llx failed!\n", __FUNCTION__, from);
		goto out;
	}

	if (r_len != len)
		printk("%s: read len (%u) is not equal to requested len (%u)\n", __FUNCTION__, r_len, len);
out:
	put_mtd_device(mtd);
	return ret;
}
EXPORT_SYMBOL(ra_mtd_read_nm);

#if defined (CONFIG_MTD_NOR_RALINK)

#define NUM_FLASH_BANKS		1
static struct mtd_info *ralink_mtd[NUM_FLASH_BANKS];
static struct map_info ralink_map[] = {
	{
		.name = "Ralink SoC physically mapped flash",
		.bankwidth = CONFIG_RT2880_MTD_PHYSMAP_BUSWIDTH,
		.size = CONFIG_RT2880_MTD_PHYSMAP_LEN,
		.phys = CPHYSADDR(CONFIG_RT2880_MTD_PHYSMAP_START)
	}
};

static int __init rt2880_mtd_init(void)
{
	int ret = -ENXIO;
	int i, found = 0;
	uint64_t flash_size = IMAGE1_SIZE;
	uint32_t kernel_size = 0x150000;
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	char *ptr;
	_ihdr_t hdr;
#endif

	if (ra_check_flash_type() != BOOT_FROM_NOR)
		return 0;

	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		printk(KERN_NOTICE "ralink flash device: 0x%x at 0x%x\n",
			(unsigned int)ralink_map[i].size, ralink_map[i].phys);

		ralink_map[i].virt = ioremap_nocache(ralink_map[i].phys, ralink_map[i].size);
		if (!ralink_map[i].virt) {
			printk("%s: failed to ioremap 0x%08x\n", __FUNCTION__, ralink_map[i].phys);
			return -EIO;
		}
		simple_map_init(&ralink_map[i]);

		ralink_mtd[i] = do_map_probe("cfi_probe", &ralink_map[i]);
		if (ralink_mtd[i]) {
			ralink_mtd[i]->owner = THIS_MODULE;
			++found;
		}
		else
			iounmap(ralink_map[i].virt);
	}

#if defined (CONFIG_RT2880_FLASH_AUTO)
	if (ralink_mtd[0])
		flash_size = ralink_mtd[0]->size;
#endif
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH) && defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	ptr = (char *)CKSEG1ADDR(CONFIG_RT2880_MTD_PHYSMAP_START + MTD_KERNEL_PART_OFFSET);
	memcpy(&hdr, ptr, sizeof(hdr));
	if (hdr.ih_ksz != 0)
		kernel_size = ntohl(hdr.ih_ksz);
#endif

	/* calculate partition table */
	recalc_partitions(flash_size, kernel_size);

	/* register the partitions */
	if (found == NUM_FLASH_BANKS) {
		ret = add_mtd_partitions(ralink_mtd[0], rt2880_partitions, ARRAY_SIZE(rt2880_partitions));
		if (ret) {
			for (i = 0; i < NUM_FLASH_BANKS; i++)
				iounmap(ralink_map[i].virt);
			return ret;
		}
	} else {
		printk("%s: error: %d flash device was found\n", __FUNCTION__, found);
		return -ENXIO;
	}

	return 0;
}

static void __exit rt2880_mtd_exit(void)
{
	int i;

	for (i = 0; i < NUM_FLASH_BANKS; i++) {
		if (ralink_mtd[i])
			map_destroy(ralink_mtd[i]);
		if (ralink_map[i].virt) {
			iounmap(ralink_map[i].virt);
			ralink_map[i].virt = NULL;
		}
	}
}

module_init(rt2880_mtd_init);
module_exit(rt2880_mtd_exit);

MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_DESCRIPTION("Ralink MTD driver");
MODULE_LICENSE("GPL");

#endif
