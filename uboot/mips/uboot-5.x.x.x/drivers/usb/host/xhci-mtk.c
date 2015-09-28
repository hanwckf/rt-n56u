#include <common.h>

#include "xhci-mtk.h"
#include "xhci.h"
#include "mtk-phy.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/**
 * Contains pointers to register base addresses
 * for the usb controller.
 */

void reinitIP(void)
{
	enableAllClockPower();
	mtk_xhci_scheduler_init();
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	u3phy_init();
	u2_slew_rate_calibration(u3phy);
	u2_slew_rate_calibration(u3phy_p1);

	mt7621_phy_init(u3phy);

        reinitIP();

	*hccr = (uint32_t)XHC_IO_START;
	*hcor = (struct xhci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("mtk-xhci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void xhci_hcd_stop(int index)
{
	disablePortClockPower();
}
