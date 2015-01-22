#ifndef __LINUX_USB_PCI_QUIRKS_H
#define __LINUX_USB_PCI_QUIRKS_H

#if defined (CONFIG_PCI) && !defined (CONFIG_MTK_XHCI)
void uhci_reset_hc(struct pci_dev *pdev, unsigned long base);
int uhci_check_and_reset_hc(struct pci_dev *pdev, unsigned long base);
int usb_amd_find_chipset_info(void);
void usb_amd_dev_put(void);
void usb_amd_quirk_pll_disable(void);
void usb_amd_quirk_pll_enable(void);
#if !defined(CONFIG_PCI_DISABLE_COMMON_QUIRKS)
bool usb_is_intel_switchable_xhci(struct pci_dev *pdev);
void usb_enable_xhci_ports(struct pci_dev *xhci_pdev);
void usb_disable_xhci_ports(struct pci_dev *xhci_pdev);
#else
static inline bool usb_is_intel_switchable_xhci(struct pci_dev *pdev)
{
	return false;
}
static inline void usb_enable_xhci_ports(struct pci_dev *xhci_pdev) {}
static inline void usb_disable_xhci_ports(struct pci_dev *xhci_pdev) {}
#endif
#else
static inline void usb_amd_quirk_pll_disable(void) {}
static inline void usb_amd_quirk_pll_enable(void) {}
static inline void usb_amd_dev_put(void) {}
static inline void usb_disable_xhci_ports(struct pci_dev *xhci_pdev) {}
#endif  /* CONFIG_PCI */

#endif  /*  __LINUX_USB_PCI_QUIRKS_H  */
