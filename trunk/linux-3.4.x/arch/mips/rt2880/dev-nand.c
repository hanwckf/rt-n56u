
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#include <asm/irq.h>
#include <asm/rt2880/rt_mmap.h>

#include <ralink/mtk_nand_dev.h>

#define NFI_BASE 	RALINK_NAND_CTRL_BASE
#define NFIECC_BASE	RALINK_NANDECC_CTRL_BASE

static struct resource mt7621_nand_resource[] = {
	{
		.start	= NFI_BASE,
		.end	= NFI_BASE + 0x1A0,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= NFIECC_BASE,
		.end	= NFIECC_BASE + 0x150,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= SURFBOARDINT_NAND,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= SURFBOARDINT_NAND_ECC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct mtk_nand_host_hw mt7621_nand_hw = {
	.nfi_bus_width		= 8,
	.nfi_cs_num		= NFI_CS_NUM,
	.nfi_cs_id		= NFI_DEFAULT_CS,
};

static struct platform_device mt7621_nand_device = {
	.name		= MTK_NAND_DRV_NAME,
	.id		= 0,
	.dev = {
		.platform_data = &mt7621_nand_hw,
	},
	.num_resources	= ARRAY_SIZE(mt7621_nand_resource),
	.resource	= mt7621_nand_resource,
};

int __init mtk_nand_register(void)
{
	int retval = 0;

	retval = platform_device_register(&mt7621_nand_device);
	if (retval != 0) {
		printk(KERN_ERR "register %s device fail!\n", "NAND");
		return retval;
	}

	return retval;
}

arch_initcall(mtk_nand_register);
