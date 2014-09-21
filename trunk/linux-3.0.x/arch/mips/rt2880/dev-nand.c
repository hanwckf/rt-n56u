#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/i2c/at24.h>
#include <linux/i2c.h>
#include <asm/rt2880/rt_mmap.h>
#include <asm/rt2880/surfboardint.h>
#include <linux/mtd/mt6575_typedefs.h>

#if defined (CONFIG_MTD_NAND_MTK)

#define NFI_base			RALINK_NAND_CTRL_BASE
#define NFIECC_base			RALINK_NANDECC_CTRL_BASE
#define MT7621_NFI_IRQ_ID		SURFBOARDINT_NAND
#define MT7621_NFIECC_IRQ_ID		SURFBOARDINT_NAND_ECC

static struct resource mt7621_resource_nand[] = {
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
		.start	= MT7621_NFI_IRQ_ID,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MT7621_NFIECC_IRQ_ID,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device mt7621_nand_dev = {
	.name		= "MT7621-NAND",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(mt7621_resource_nand),
	.resource	= mt7621_resource_nand,
	.dev = {
		.platform_data = &mt7621_nand_hw,
	},
};

int __init mtk_nand_register(void)
{
	int retval = 0;

	retval = platform_device_register(&mt7621_nand_dev);
	if (retval != 0) {
		printk(KERN_ERR "register nand device fail\n");
		return retval;
	}

	return retval;
}
arch_initcall(mtk_nand_register);

#endif
