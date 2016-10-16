
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#if defined(CONFIG_USB_XHCI_PLATFORM)
#include <linux/usb/xhci_pdriver.h>
#endif

#include <asm/irq.h>
#include <asm/rt2880/rt_mmap.h>

#define MT7621_XHCI_MEM_START	RALINK_USB_HOST_BASE

static struct resource mt7621_xhci_resources[] = {
	[0] = {
		.start  = MT7621_XHCI_MEM_START,
		.end    = MT7621_XHCI_MEM_START + 0xffff,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SURFBOARDINT_USB,
		.flags  = IORESOURCE_IRQ,
	},
};

static u64 mt7621_xhci_dmamask = DMA_BIT_MASK(32);

#if defined(CONFIG_USB_XHCI_PLATFORM)
extern void uphy_init(void);

static void mt7621_uphy_init(struct platform_device *pdev)
{
	uphy_init();
}

/* MTK host controller gives a spurious successful event after a short transfer. Ignore it. */
static struct usb_xhci_pdata mt7621_xhci_pdata = {
	.usb3_lpm_capable	= 1,
	.spurious_success	= 1,
	.uphy_init		= mt7621_uphy_init,
};
#endif

static struct platform_device mt7621_xhci_device = {
	.name		= "xhci-hcd",
	.id		= -1,
	.dev		= {
#if defined(CONFIG_USB_XHCI_PLATFORM)
		.platform_data = &mt7621_xhci_pdata,
#endif
		.dma_mask = &mt7621_xhci_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
	.num_resources	= ARRAY_SIZE(mt7621_xhci_resources),
	.resource	= mt7621_xhci_resources,
};

int __init init_mt7621_xhci(void)
{
	int retval = 0;

	retval = platform_device_register(&mt7621_xhci_device);
	if (retval != 0) {
		printk(KERN_ERR "register %s device fail!\n", "xHCI");
		return retval;
	}

	return retval;
}

device_initcall(init_mt7621_xhci);
