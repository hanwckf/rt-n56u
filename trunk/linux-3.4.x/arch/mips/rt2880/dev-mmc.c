
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#include <linux/mmc/mmc_mtk.h>

#include <asm/rt2880/surfboardint.h>
#include <asm/rt2880/rt_mmap.h>

#define MTK_SDXC_MEM_START	RALINK_MSDC_BASE

static struct resource mtk_mmc_resources[] = {
	[0] = {
		.start  = MTK_SDXC_MEM_START,
		.end    = MTK_SDXC_MEM_START + 0x3fff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SURFBOARDINT_SDXC,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct msdc_hw mtk_mmc_hw = {
	.clk_src	= 0,
	.cmd_edge	= MSDC_SMPL_FALLING,
	.data_edge	= MSDC_SMPL_FALLING,
	.clk_drv	= 4,
	.cmd_drv	= 4,
	.dat_drv	= 4,
#if defined (CONFIG_MTK_MMC_EMMC_8BIT)
	.data_pins	= 8,
#else
	.data_pins	= 4,
#endif
	.data_offset	= 0,
	.flags		= MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE | MSDC_HIGHSPEED,
};

static struct platform_device mtk_mmc_device = {
	.name		= MMC_DRV_NAME,
	.id		= 0,
	.dev		= {
		.platform_data = &mtk_mmc_hw,
	},
	.num_resources	= ARRAY_SIZE(mtk_mmc_resources),
	.resource	= mtk_mmc_resources,
};

int __init init_mtk_mmc(void)
{
	int retval = 0;

	retval = platform_device_register(&mtk_mmc_device);
	if (retval != 0) {
		printk(KERN_ERR "register %s device fail!\n", "MMC");
		return retval;
	}

	return retval;
}

device_initcall(init_mtk_mmc);
