
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/platform_device.h>

#include <asm/irq.h>
#include <asm/rt2880/rt_mmap.h>

#include <ralink/mtk_mmc_dev.h>

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

#define CLKCFG1_REG		(RALINK_SYSCTL_BASE + 0x30)
#define RSTCTRL_REG		(RALINK_SYSCTL_BASE + 0x34)

static void mtk_mmc_power_on(void)
{
	u32 val;

	/* enable SDXC clock */
	val = le32_to_cpu(*(volatile u32 *)(CLKCFG1_REG));
	val |= (RALINK_SDXC_CLK_EN);
	*(volatile u32 *)(CLKCFG1_REG) = cpu_to_le32(val);

	mdelay(1);

	/* release SDXC reset */
	val = le32_to_cpu(*(volatile u32 *)(RSTCTRL_REG));
	val &= ~(RALINK_SDXC_RST);
	*(volatile u32 *)(RSTCTRL_REG) = cpu_to_le32(val);

	mdelay(10);
}

static void mtk_mmc_power_off(void)
{
	u32 val;

	/* raise SDXC reset */
	val = le32_to_cpu(*(volatile u32 *)(RSTCTRL_REG));
	val |= (RALINK_SDXC_RST);
	*(volatile u32 *)(RSTCTRL_REG) = cpu_to_le32(val);

	udelay(100);

	/* disable SDXC clock */
	val = le32_to_cpu(*(volatile u32 *)(CLKCFG1_REG));
	val &= ~(RALINK_SDXC_CLK_EN);
	*(volatile u32 *)(CLKCFG1_REG) = cpu_to_le32(val);
}

static struct msdc_hw mtk_mmc_hw = {
	.clk_src	= 0,
	.cmd_edge	= MSDC_SMPL_FALLING,
	.data_edge	= MSDC_SMPL_FALLING,
	.crc_edge	= MSDC_SMPL_FALLING,
	.clk_drv	= 4,
	.cmd_drv	= 4,
	.dat_drv	= 4,
#if defined (CONFIG_MTK_MMC_EMMC_8BIT)
	.data_pins	= 8,
#else
	.data_pins	= 4,
#endif
	.flags		= MSDC_SYS_SUSPEND | MSDC_WP_PIN_EN | MSDC_CD_PIN_EN | MSDC_REMOVABLE | MSDC_HIGHSPEED,
	.ext_power_on	= mtk_mmc_power_on,
	.ext_power_off	= mtk_mmc_power_off,
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
