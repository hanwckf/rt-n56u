/*
 * RT3883 OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 2009 Ralink Tech Company
 *
 * Bus Glue for Ralink OHCI controller.
 *
 * Written by YYHuang <yy_huang@ralinktech.com.tw>
 * Based on fragments of previous driver by Russell King et al.
 *
 * This file is licenced under the GPL.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/signal.h>
#include <linux/platform_device.h>

#ifndef CONFIG_RT3XXX_EHCI /* wake_up/sleep already handled by ehci-rt3xxx.c */
static void try_wake_up(void)
{
	u32 val;

	// enable port0 & port1 Phy clock
	val = le32_to_cpu(*(volatile u_long *)(0xB0000030));
	val = val | 0x00140000;
	*(volatile u_long *)(0xB0000030) = cpu_to_le32(val);
	mdelay(10);

	// toggle reset bit 25 & 22 to 0
	val = le32_to_cpu(*(volatile u_long *)(0xB0000034));
	val = val & 0xFDBFFFFF;
	*(volatile u_long *)(0xB0000034) = cpu_to_le32(val);
	mdelay(100);
}

static void try_sleep(void)
{
	u32 val;

	// toggle reset bit 25 & 22 to 1
	val = le32_to_cpu(*(volatile u_long *)(0xB0000034));
	val = val | 0x02400000;
	*(volatile u_long *)(0xB0000034) = cpu_to_le32(val);
	mdelay(10);

	// disable port0 & port1 Phy clock
	val = le32_to_cpu(*(volatile u_long *)(0xB0000030));
	val = val & 0xFFEBFFFF;
	*(volatile u_long *)(0xB0000030) = cpu_to_le32(val);
	mdelay(10);
}
#endif

static int ohci_rt3xxx_start(struct usb_hcd *hcd)
{
	int ret;
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	ret = ohci_run(ohci);
	if (ret < 0)
		goto err;

	return 0;

err:
	ohci_stop(hcd);
	return ret;
}

static struct hc_driver rt3xxx_ohci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Ralink RT3xxx OHCI Controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),
	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,

	.start			= ohci_rt3xxx_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,

	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	.get_frame_number	= ohci_get_frame,

	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
	.start_port_reset	= ohci_start_port_reset,
};

static int rt3xxx_ohci_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
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

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Found HC with no register addr.\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(&rt3xxx_ohci_hc_driver, &pdev->dev, "rt3xxx-ohci");
	if (!hcd) {
		retval = -ENOMEM;
		goto fail_create_hcd;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto fail_request_resource;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto fail_ioremap;
	}

#ifndef CONFIG_RT3XXX_EHCI /* wake_up/sleep already handled by ehci-rt3xxx.c */
	try_wake_up();
#endif

	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
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
	dev_err(&pdev->dev, "RT3xxx OHCI init fail. %d\n", retval);
	return retval;
}

static int rt3xxx_ohci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

#ifndef CONFIG_RT3XXX_EHCI /* wake_up/sleep already handled by ehci-rt3xxx.c */
	try_sleep();
#endif

	return 0;
}

static struct platform_driver rt3xxx_ohci_driver = {
	.probe		= rt3xxx_ohci_probe,
	.remove		= rt3xxx_ohci_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "rt3xxx-ohci",
	},
};

MODULE_ALIAS("rt3xxx-ohci");
