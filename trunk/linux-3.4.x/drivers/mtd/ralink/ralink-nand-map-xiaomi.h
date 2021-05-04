#ifndef __RALINK_NAND_MAP_XIAOMI_H__
#define __RALINK_NAND_MAP_XIAOMI_H__

/*
 * mi-3:	uboot(256k),config(256k),bdata(256k),factory(256k),crash(256k),crash_syslog(256k),reserved0(512k),kernel0(4096k),kernel1(4096k),rootfs+overlay(-)
 * mi-r3p:	uboot(256k),config(256k),bdata(256k),factory(256k),crash(256k),crash_syslog(512k),cfg_bak(256k),kernel0(4096k),kernel1(4096k),rootfs+overlay(-)
 * mi-r3g:	uboot(512k),config(256k),bdata(256k),factory(256k),crash(256k),crash_syslog(256k),reserved0(256k),kernel0(4096k),kernel1(4096k),rootfs+overlay(-)
 * rm2100:	uboot(512k),config(256k),bdata(256k),factory(256k),crash(256k),crash_syslog(256k),reserved0(256k),kernel0(4096k),kernel1(4096k),rootfs+overlay+obr(-)
 * cr660x:	uboot(512k),nvram(256k),bdata(256k),factory(512k),crash(256k),crash_syslog(256k),firmware(30720k),firmware1(30720k),overlay+obr(-)
*/

#define NAND_MTD_BOOT_PART_SIZE	CONFIG_MTD_BOOTLOADER_PART_SIZ
#define NAND_MTD_CFG_ENV_PART_SIZE	CONFIG_MTD_BOOTENV_PART_SIZ
#define NAND_MTD_FACTORY_PART_SIZE	CONFIG_MTD_FACTORY_PART_SIZ
#define NAND_MTD_RESERVED_PART_SIZE	CONFIG_MTD_RESERVED_PART_SIZ
#define NAND_MTD_KERNEL_PART_SIZE	CONFIG_MTD_FIRMWARE_PART_SIZ
#define NAND_MTD_CONFIG_PART_SIZE	CONFIG_MTD_CONFIG_PART_SIZ
#define NAND_MTD_STORE_PART_SIZE	CONFIG_MTD_STORE_PART_SIZ

#define NAND_MTD_KERNEL_PART_IDX	4
#define NAND_MTD_KERNEL_PART_OFFSET	(NAND_MTD_BOOT_PART_SIZE + NAND_MTD_CFG_ENV_PART_SIZE + NAND_MTD_FACTORY_PART_SIZE + NAND_MTD_RESERVED_PART_SIZE)
#define NAND_MTD_RWFS_PART_OFFSET	(NAND_MTD_KERNEL_PART_OFFSET + NAND_MTD_KERNEL_PART_SIZE + NAND_MTD_CONFIG_PART_SIZE + NAND_MTD_STORE_PART_SIZE)

#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
#define NAND_MTD_ROOTFS_PART_IDX	(NAND_MTD_KERNEL_PART_IDX + 1)
#define NAND_MTD_RWFS_PART_IDX		8
#else
#define NAND_MTD_RWFS_PART_IDX		7
#endif

#define NAND_MTD_ALL_PART_IDX (NAND_MTD_RWFS_PART_IDX + 2)

static struct mtd_partition rt2880_partitions[] = {
	{
		/* mtd0 */
		name:   "Bootloader",
		size:   NAND_MTD_BOOT_PART_SIZE,
		offset: 0,
	}, {
		/* mtd1 */
		/* Config/Nvram + Bdata */
		name:   "CFG_Bdata",
		size:   NAND_MTD_CFG_ENV_PART_SIZE,
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd2 */
		name:   "Factory",
		size:   NAND_MTD_FACTORY_PART_SIZE,
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd3 */
		/* crash + crash_log + reserved0/cfg_bak(optional) + kernel0(optional) */
		name:   "Reserved",
		size:   NAND_MTD_RESERVED_PART_SIZE,
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd4 */
		/* kernel1 */
		name:   "Kernel",
		size:   NAND_MTD_KERNEL_PART_SIZE,
		offset: NAND_MTD_KERNEL_PART_OFFSET,
#if defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
	}, {
		/* mtd5 */
		name:   "RootFS",
		size:   0,				/* calc */
		offset: MTDPART_OFS_APPEND,
#endif
	}, {
		/* mtd6 */
		name:   "Config",
		size:   NAND_MTD_CONFIG_PART_SIZE,
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd7 */
		name:   "Storage",
		size:   NAND_MTD_STORE_PART_SIZE,
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd8 */
		name:   "RWFS",
		size:   0,				/* calc */
		offset: MTDPART_OFS_APPEND,
	}, {
		/* mtd9 */
		name:   "Firmware_Stub",
		size:   NAND_MTD_KERNEL_PART_SIZE,	/* Kernel+RootFS */
		offset: NAND_MTD_KERNEL_PART_OFFSET,
	}, {
		/* mtd10 */
		name:	"ALL",
		size:	0,
		offset: 0,
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
	rt2880_partitions[NAND_MTD_ROOTFS_PART_IDX].size = NAND_MTD_KERNEL_PART_SIZE - rt2880_partitions[NAND_MTD_KERNEL_PART_IDX].size;
#endif

	/* calc "RWFS" size (UBIFS or JFFS2) */
	rt2880_partitions[NAND_MTD_RWFS_PART_IDX].size = flash_size - NAND_MTD_RWFS_PART_OFFSET;

	rt2880_partitions[NAND_MTD_ALL_PART_IDX].size = flash_size;
}

#endif
