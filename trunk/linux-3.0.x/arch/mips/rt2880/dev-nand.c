
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#include <linux/mtd/mt6575_typedefs.h>

#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>

#define NFI_base		RALINK_NAND_CTRL_BASE
#define NFIECC_base		RALINK_NANDECC_CTRL_BASE

static struct resource mt7621_nand_resource[] = {
	{
		.start	= NFI_base,
		.end	= NFI_base + 0x1A0,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= NFIECC_base,
		.end	= NFIECC_base + 0x150,
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

static struct platform_device mt7621_nand_device = {
	.name		= "MT7621-NAND",
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
