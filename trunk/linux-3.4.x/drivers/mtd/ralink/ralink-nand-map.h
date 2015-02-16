#ifndef __RALINK_NAND_MAP_H__
#define __RALINK_NAND_MAP_H__

#if defined (CONFIG_MTD_NAND_USE_CUSTOM_PART)
#define NAND_MTD_BOOT_PART_SIZE		CONFIG_MTD_BOOTLOADER_PART_SIZ
#define NAND_MTD_CONFIG_PART_SIZE	CONFIG_MTD_CONFIG_PART_SIZ
#define NAND_MTD_FACTORY_PART_SIZE	CONFIG_MTD_FACTORY_PART_SIZ
#else
#define NAND_MTD_BOOT_PART_SIZE		0x00080000	/* 4x NAND blocks */
#define NAND_MTD_CONFIG_PART_SIZE	0x00080000	/* 4x NAND blocks */
#define NAND_MTD_FACTORY_PART_SIZE	0x00040000	/* 2x NAND blocks */
#endif

#define NAND_MTD_KERNEL_PART_SIZE	CONFIG_MTD_FIRMWARE_PART_SIZ
#define NAND_MTD_STORE_PART_SIZE	CONFIG_MTD_STORE_PART_SIZ

#define NAND_MTD_KERNEL_PART_IDX	3

#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
#define NAND_MTD_ROOTFS_PART_IDX	4
#define NAND_MTD_RWFS_PART_IDX		6
#else
#define NAND_MTD_RWFS_PART_IDX		5
#endif

#define NAND_MTD_KERNEL_PART_OFFSET	(NAND_MTD_BOOT_PART_SIZE + NAND_MTD_CONFIG_PART_SIZE + NAND_MTD_FACTORY_PART_SIZE)
#define NAND_MTD_RWFS_PART_OFFSET	(NAND_MTD_KERNEL_PART_OFFSET + NAND_MTD_KERNEL_PART_SIZE + NAND_MTD_STORE_PART_SIZE)

static struct mtd_partition rt2880_partitions[] = {
	{
		name:   "Bootloader",			/* mtdblock0 */
		size:   NAND_MTD_BOOT_PART_SIZE,	/* 512K */
		offset: 0,
	}, {
		name:   "Config",			/* mtdblock1 */
		size:   NAND_MTD_CONFIG_PART_SIZE,	/* 256K */
		offset: MTDPART_OFS_APPEND,
	}, {
		name:   "Factory",			/* mtdblock2 */
		size:   NAND_MTD_FACTORY_PART_SIZE,	/* 256K */
		offset: MTDPART_OFS_APPEND,
	}, {
		name:   "Kernel",			/* mtdblock3 */
		size:   NAND_MTD_KERNEL_PART_SIZE,	/* 32M */
		offset: NAND_MTD_KERNEL_PART_OFFSET,
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
	}, {
		name:   "RootFS",			/* mtdblock4 */
		size:   0,				/* calc */
		offset: MTDPART_OFS_APPEND,
#endif
	}, {
		name:   "Storage",			/* mtdblock5 */
		size:   NAND_MTD_STORE_PART_SIZE,	/* 2M */
		offset: MTDPART_OFS_APPEND,
	}, {
		name:   "RWFS",				/* mtdblock6 */
		size:   0,				/* calc */
		offset: MTDPART_OFS_APPEND,
	}, {
		name:   "Firmware_Stub",		/* mtdblock7 */
		size:   NAND_MTD_KERNEL_PART_SIZE,	/* Kernel+RootFS */
		offset: NAND_MTD_KERNEL_PART_OFFSET,
	}
};

inline void recalc_partitions(uint64_t flash_size, uint32_t kernel_size)
{
	/* calc "Kernel" size */
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
#if defined (CONFIG_ROOTFS_IN_FLASH_NO_PADDING)
	rt2880_partitions[NAND_MTD_KERNEL_PART_IDX].size = kernel_size;
#else
	rt2880_partitions[NAND_MTD_KERNEL_PART_IDX].size = CONFIG_MTD_KERNEL_PART_SIZ;
#endif
	/* calc "RootFS" size */
	rt2880_partitions[NAND_MTD_ROOTFS_PART_IDX].size = NAND_MTD_KERNEL_PART_SIZE - rt2880_partitions[3].size;
#endif

	/* calc "RWFS" size (UBIFS or JFFS2) */
	rt2880_partitions[NAND_MTD_RWFS_PART_IDX].size = flash_size - NAND_MTD_RWFS_PART_OFFSET;
}

#endif
