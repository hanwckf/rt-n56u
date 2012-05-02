/* vi: set sw=4 ts=4: */
/*
 * elbox_physmap.c
 *
 * Copyright (C) 2008 Alpha Networks, Inc.
 * by David Hsieh <david_hsieh@alphanetworks.com>
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <linux/squashfs_fs.h>

#ifndef CONFIG_MTD_PARTITIONS
#error "MTD partition should be selected !!"
#endif

struct elbox_flash_info
{
	struct mtd_info *		mtd;
	struct map_info			map;
	struct resource *		res;
	//int						nr_parts;
	//struct mtd_partition *	parts;
};

struct elbox_flash_data
{
	unsigned int			width;
	unsigned int			nr_parts;
	struct mtd_partition *	parts;
};

static struct mtd_partition * init_mtd_partitions(struct mtd_info * mtd, size_t size);

static int physmap_flash_remove(struct platform_device *dev)
{
	struct elbox_flash_info *info;
	struct elbox_flash_data *physmap_data;

	info = platform_get_drvdata(dev);
	if (info == NULL) return 0;
	platform_set_drvdata(dev, NULL);

	physmap_data = dev->dev.platform_data;

	if (info->mtd != NULL)
	{
		if (physmap_data->nr_parts) del_mtd_partitions(info->mtd);
		del_mtd_device(info->mtd);
		map_destroy(info->mtd);
	}

	if (info->map.virt != NULL)
		iounmap(info->map.virt);

	if (info->res != NULL)
	{
		release_resource(info->res);
		kfree(info->res);
	}

	kfree(info);
	return 0;
}

static int physmap_flash_probe(struct platform_device *dev)
{
	struct elbox_flash_data *physmap_data;
	struct elbox_flash_info *info;
	int err;

	physmap_data = dev->dev.platform_data;
	if (physmap_data == NULL) return -ENODEV;

	printk(KERN_NOTICE "elbox physmap platform flash device: %.8llx at %.8llx\n",
		(unsigned long long)(dev->resource->end - dev->resource->start + 1),
		(unsigned long long)dev->resource->start);

	info = kzalloc(sizeof(struct elbox_flash_info), GFP_KERNEL);
	if (info == NULL)
	{
		err = -ENOMEM;
		goto err_out;
	}

	platform_set_drvdata(dev, info);

	info->res = request_mem_region(dev->resource->start,
			dev->resource->end - dev->resource->start + 1,
			dev->dev.bus_id);
	if (info->res == NULL)
	{
		dev_err(&dev->dev, "Could not reserve memory region\n");
		err = -ENOMEM;
		goto err_out;
	}

	info->map.name = dev->dev.bus_id;
	info->map.phys = dev->resource->start;
	info->map.size = dev->resource->end - dev->resource->start + 1;
	info->map.bankwidth = physmap_data->width;

	info->map.virt = ioremap(info->map.phys, info->map.size);
	if (info->map.virt == NULL)
	{
		dev_err(&dev->dev, "Failed to ioremap flash region\n");
		err = EIO;
		goto err_out;
	}

	simple_map_init(&info->map);

	info->mtd = do_map_probe("cfi_probe", &info->map);
	if (info->mtd == NULL)
	{
		dev_err(&dev->dev, "map_probe failed\n");
		err = -ENXIO;
		goto err_out;
	}
	info->mtd->owner = THIS_MODULE;
	add_mtd_device(info->mtd);
	if (init_mtd_partitions(info->mtd, info->map.size) && physmap_data->nr_parts)
		add_mtd_partitions(info->mtd, physmap_data->parts, physmap_data->nr_parts);

	return 0;

err_out:
	physmap_flash_remove(dev);
	return err;
}

#ifdef CONFIG_PM
static int physmap_flash_suspend(struct platform_device *dev, pm_message_t state)
{
	struct elbox_flash_info *info = platform_get_drvdata(dev);
	int ret = 0;
	if (info) ret = info->mtd->suspend(info->mtd);
	return ret;
}

static int physmap_flash_resume(struct platform_device *dev)
{
	struct elbox_flash_info *info = platform_get_drvdata(dev);
	if (info) info->mtd->resume(info->mtd);
	return 0;
}

static void physmap_flash_shutdown(struct platform_device *dev)
{
	struct elbox_flash_info *info = platform_get_drvdata(dev);
	if (info && info->mtd->suspend(info->mtd) == 0)
		info->mtd->resume(info->mtd);
}
#endif

/*********************************************************************/

static struct platform_driver elbox_flash_driver =
{
	.probe		= physmap_flash_probe,
	.remove		= physmap_flash_remove,
#ifdef CONFIG_PM
	.suspend	= physmap_flash_suspend,
	.resume		= physmap_flash_resume,
	.shutdown	= physmap_flash_shutdown,
#endif
	.driver		=
	{
		.name	= "elbox-flash",
	},
};

/*********************************************************************/

/*
 *  Flash layout
 *
 *  |      16K      |      16K      |      16K      |      16K      |
 *  +---------------------------------------------------------------+ 0x00000000
 *  |                                                               |
 *  | U-Boot (64K x 3)                                              |
 *  |                                                               |
 *  +---------------------------------------------------------------+ 0x00030000
 *  | u-boot config                 | devdata (include RF)          |
 *  +---------------------------------------------------------------+ 0x00040000
 *  | devconf (64K x 1)                                             |
 *  +---------------------------------------------------------------+ 0x00050000
 *  |                                                               |
 *  | Upgrade (linux kernel/rootfs)                                 |
 *  ~                                                               ~
 *  |                                                               |
 *  |                                                               |
 *  +---------------------------------------------------------------+ 0x003f0000
 *  | Language pack (64K x 1)                                       |
 *  +---------------------------------------------------------------+ 0x00400000
 */

#define FLASH_SIZE		CONFIG_MTD_ELBOX_PHYSMAP_LEN
#ifdef CONFIG_MTD_ELBOX_LANG_PACK
#define LANGPACK_SIZE	CONFIG_MTD_ELBOX_LANG_PACK_SIZE
#else
#define LANGPACK_SIZE	0
#endif
#define BOOTCODE_SIZE	0x30000
#define DEVCONF_SIZE	0x10000
#define DEVDATA_SIZE	0x10000
#define UPGRADE_SIZE	(FLASH_SIZE - BOOTCODE_SIZE - DEVCONF_SIZE - DEVDATA_SIZE - LANGPACK_SIZE)

#define DEVDATA_OFFSET	(BOOTCODE_SIZE)
#define DEVCONF_OFFSET	(BOOTCODE_SIZE + DEVDATA_SIZE)
#define UPGRADE_OFFSET	(BOOTCODE_SIZE + DEVDATA_SIZE + DEVCONF_SIZE)
#define LANGPACK_OFFSET	(BOOTCODE_SIZE + DEVDATA_SIZE + DEVCONF_SIZE + UPGRADE_SIZE)

static struct mtd_partition elbox_partitions[] =
{
	/* The following partitions are the "MUST" in ELBOX. */
	{name:"rootfs",		offset:0,				size:0,				mask_flags:MTD_WRITEABLE, },
	{name:"upgrade",	offset:UPGRADE_OFFSET,	size:UPGRADE_SIZE,	},
	{name:"devconf",	offset:DEVCONF_OFFSET,	size:DEVCONF_SIZE,	},
	{name:"devdata",	offset:DEVDATA_OFFSET,	size:DEVDATA_SIZE,	},
	{name:"langpack",	offset:LANGPACK_OFFSET,	size:LANGPACK_SIZE,	},
	{name:"flash",		offset:0,				size:FLASH_SIZE,	mask_flags:MTD_WRITEABLE, },

	/* The following partitions are board dependent. */
	{name:"u-boot",		offset:0,				size:BOOTCODE_SIZE,	mask_flags:MTD_WRITEABLE, },
	{name:"boot env",	offset:DEVDATA_OFFSET,	size:0x8000,		mask_flags:MTD_WRITEABLE, }
};

/*********************************************************************/
/* SEAMA */
#define SEAMA_MAGIC 0x5EA3A417
typedef struct seama_hdr seamahdr_t;
struct seama_hdr
{
	uint32_t	magic;		/* should always be SEAMA_MAGIC. */
	uint16_t	reserved;	/* reserved for  */
	uint16_t	metasize;	/* size of the META data */
	uint32_t	size;		/* size of the image */
} __attribute__ ((packed));

/* the tag is 32 bytes octet,
 * first part is the tag string,
 * and the second half is reserved for future used. */
#define PACKIMG_TAG "--PaCkImGs--"
struct packtag
{
	char tag[16];
	unsigned long size;
	char reserved[12];
};

static struct mtd_partition * init_mtd_partitions(struct mtd_info * mtd, size_t size)
{
	struct squashfs_super_block * squashfsb;
	struct packtag * ptag = NULL;
	unsigned char buf[64];
	int off = elbox_partitions[1].offset;
	size_t len;
	seamahdr_t * seama;

	/* Try to read the SEAMA header */
	memset(buf, 0xa5, sizeof(buf));
	if ((mtd->read(mtd, off, sizeof(seamahdr_t), &len, buf) == 0)
		&& (len == sizeof(seamahdr_t)))
	{
		seama = (seamahdr_t *)buf;
		if (ntohl(seama->magic) == SEAMA_MAGIC)
		{
			/* We got SEAMA, the offset should be shift. */
			off += sizeof(seamahdr_t);
			if (ntohl(seama->size) > 0) off += 16;
			off += ntohs(seama->metasize);
			printk("%s: the flash image has SEAMA header\n",mtd->name);
		}
	}

	/* Looking for PACKIMG_TAG in the 64K boundary. */
	for (off += CONFIG_MTD_ELBOX_KERNEL_SKIP; off < size; off += (64*1024))
	{
		/* Find the tag. */
		memset(buf, 0xa5, sizeof(buf));
		if (mtd->read(mtd, off, sizeof(buf), &len, buf) || len != sizeof(buf)) continue;
		if (memcmp(buf, PACKIMG_TAG, 12)) continue;
		/* We found the tag, check for the supported file system. */
		squashfsb = (struct squashfs_super_block *)(buf + sizeof(struct packtag));
		if (squashfsb->s_magic == SQUASHFS_MAGIC_LZMA)
		{
			printk(KERN_NOTICE "%s: squashfs filesystem found at offset %d\n", mtd->name, off);
			ptag = (struct packtag *)buf;
			elbox_partitions[0].offset = off + 32;
			elbox_partitions[0].size = ntohl(ptag->size);
			return elbox_partitions;
		}
	}

	printk(KERN_NOTICE "%s: Couldn't find valid rootfs image!\n", mtd->name);
	return NULL;
}

/*********************************************************************/

static struct elbox_flash_data flash_data =
{
	.width	= CONFIG_MTD_ELBOX_PHYSMAP_BANKWIDTH,
	.parts	= elbox_partitions,
	.nr_parts = sizeof(elbox_partitions)/sizeof(struct mtd_partition),
};

static struct resource flash_resource =
{
	.start	= CONFIG_MTD_ELBOX_PHYSMAP_START,
	.end	= CONFIG_MTD_ELBOX_PHYSMAP_START + CONFIG_MTD_ELBOX_PHYSMAP_LEN - 1,
	.flags	= IORESOURCE_MEM,
};

static struct platform_device elbox_flash =
{
	.name	= "elbox-flash",
	.id		= 0,
	.dev	=
	{
		.platform_data	= &flash_data,
	},
	.num_resources	= 1,
	.resource	= &flash_resource,
};

/*********************************************************************/

static int __init elbox_physmap_init(void)
{
	int err;

	err = platform_driver_register(&elbox_flash_driver);
	if (err == 0) platform_device_register(&elbox_flash);
	return err;
}

static void __exit elbox_physmap_exit(void)
{
	platform_device_unregister(&elbox_flash);
	platform_driver_unregister(&elbox_flash_driver);
}

module_init(elbox_physmap_init);
module_exit(elbox_physmap_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Hsieh <david_hsieh@alphanetworks.com>");
MODULE_DESCRIPTION("MTD map driver for ELBOX");
