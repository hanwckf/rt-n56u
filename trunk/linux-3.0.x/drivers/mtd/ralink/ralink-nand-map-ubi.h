#ifndef __RALINK_NAND_MAP_UBI_H__
#define __RALINK_NAND_MAP_UBI_H__

#define NAND_MTD_BOOT_PART_SIZE		CONFIG_MTD_BOOTLOADER_PART_SIZ

#define NAND_MTD_UBI_PART_OFFSET	NAND_MTD_BOOT_PART_SIZE

static struct mtd_partition rt2880_partitions[] = {
	{
		name:   "Bootloader",			/* mtdblock0 */
		size:   NAND_MTD_BOOT_PART_SIZE,	/* 1M */
		offset: 0,
	}, {
		name:   "UBI_DEV",			/* mtdblock1 (UBI root) */
		size:   0,				/* calc */
		offset: NAND_MTD_UBI_PART_OFFSET,
	}
};

inline void recalc_partitions(uint64_t flash_size)
{
	/* calc "UBI_DEV" size */
	rt2880_partitions[1].size = flash_size - rt2880_partitions[1].offset;
}

#endif

