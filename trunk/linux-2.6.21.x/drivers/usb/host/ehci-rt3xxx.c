/*
 * Ralink 3XXX(3883) EHCI Host Controller Driver
 *
 * Author: Ying Yuan Huang <yyhuang@ralinktech.com.tw>
 * Based on "ehci-fsl.c" by Randy Vinson <rvinson@mvista.com>
 *
 * 2009 (c) Ralink Technology, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <linux/platform_device.h>
#include <asm/rt2880/rt_mmap.h>

#define SYSCFG1			(RALINK_SYSCTL_BASE + 0x14)
#define USB0_HOST_MODE	(1UL<<10)

#define RSTCTRL 		(RALINK_SYSCTL_BASE + 0x34)
#define UHST_RST		(1UL<<22)

#define USB_PS			(RALINK_SYSCTL_BASE + 0x5C)
#define USB_PS_PORT0	(1UL<<0)
#define USB_PS_PORT1	(1UL<<1)

void static inline rt_writel(u32 val, unsigned long reg)
{
    *(volatile u32 *)(reg) = val;
}

static inline u32 rt_readl(unsigned long reg)
{
    return (*(volatile u32 *)reg);
}

static int rt_set_host(void)
{
	u32 val = rt_readl(SYSCFG1);
	// host mode
	val |= USB0_HOST_MODE;
	rt_writel(val, SYSCFG1);

	return 0;
}

static int rt_usbhost_reset(void)
{
	u32 val = rt_readl(RSTCTRL);
	val |= UHST_RST;
	rt_writel(val, RSTCTRL);
	val &= ~(UHST_RST);
	rt_writel(val, RSTCTRL);
	mdelay (100);

	return 0;
}

static int rt3xxx_ehci_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval = 0;

	/* No USB-PCI space. */
	ehci->caps = hcd->regs /* + 0x100 */;
	ehci->regs = hcd->regs /* + 0x100 */ + HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	ehci_reset(ehci);

	retval = ehci_init(hcd);
	if (retval)
		return retval;

	ehci_port_power(ehci, 0);

	return retval;
}

static const struct hc_driver rt3xxx_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Ralink EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,
	.reset			= rt3xxx_ehci_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.get_frame_number	= ehci_get_frame,
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
};

static void try_wake_up(void)
{
	u32 val;

	val = le32_to_cpu(*(volatile u_long *)(0xB0000030));
	//if(val & 0x00040000)
	//	return;		// Someone(OHCI?) has waked it up, then just return.
	val = val | 0x00140000;
	*(volatile u_long *)(0xB0000030) = cpu_to_le32(val);
	udelay(10000);	// enable port0 & port1 Phy clock

	val = le32_to_cpu(*(volatile u_long *)(0xB0000034));
	val = val & 0xFDBFFFFF;
	*(volatile u_long *)(0xB0000034) = cpu_to_le32(val);
	udelay(10000);	// toggle reset bit 25 & 22 to 0
}

static int rt3xxx_ehci_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	const struct hc_driver *driver = &rt3xxx_ehci_hc_driver;
	struct resource *res;
	int irq;
	int retval;


	if (usb_disabled())
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev, "Found HC with no IRQ.\n");
		return -ENODEV;
	}
	irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev, "rt3xxx" /*dev_name(&pdev->dev)*/);
	if (!hcd) {
		retval = -ENOMEM;
		goto fail_create_hcd;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,	"Found HC with no register addr.\n");
		retval = -ENODEV;
		goto fail_request_resource;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto fail_request_resource;
	}

	hcd->regs = ioremap_nocache(hcd->rsrc_start, hcd->rsrc_len);
	if (hcd->regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto fail_ioremap;
	}

	// reset host controller
	//rt_usbhost_reset();

	// wake up usb module from power saving mode...
	try_wake_up();

#ifdef CONFIG_USB_GADGET_RT
#error	"*********************************************************"
#ifdef CONFIG_RALINK_RT5350
#error	"*    EHCI won't have any USB port to run!               *"
#else
#error	"*    EHCI will yield USB port0 to device controller!    *"
#endif /* CONFIG_RALINK_RT5350 */
#error	"*********************************************************"
#else
	// change port0 to host mode
	rt_set_host();
#endif

	retval = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (retval)
		goto fail_add_hcd;

	return retval;

fail_add_hcd:
	iounmap(hcd->regs);
fail_ioremap:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
fail_request_resource:
	usb_put_hcd(hcd);
fail_create_hcd:
	dev_err(&pdev->dev, "RT3xxx EHCI init fail. %d\n", retval);
	return retval;
}

static void try_sleep(void)
{
    u32 val;

    val = le32_to_cpu(*(volatile u_long *)(0xB0000030));
    val = val & 0xFFEBFFFF;
    *(volatile u_long *)(0xB0000030) = cpu_to_le32(val);
    udelay(10000);  // disable port0 & port1 Phy clock

    val = le32_to_cpu(*(volatile u_long *)(0xB0000034));
    val = val | 0x02400000;
    *(volatile u_long *)(0xB0000034) = cpu_to_le32(val);
    udelay(10000);  // toggle reset bit 25 & 22 to 1
}

static int rt3xxx_ehci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	/* ehci_shutdown() is supposed to be called implicitly in 
	   ehci-hcd common code while removing module, but it isn't. */
	ehci_shutdown(hcd);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	if(!usb_find_device(0x0, 0x0)) // No any other USB host controller.
		try_sleep();

	return 0;
}

MODULE_ALIAS("rt3xxx-ehci");

static struct platform_driver rt3xxx_ehci_driver = {
	.probe = rt3xxx_ehci_probe,
	.remove = rt3xxx_ehci_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		.name = "rt3xxx-ehci",
	},
};


