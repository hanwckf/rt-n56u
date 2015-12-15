#ifndef _XHCI_MTK_POWER_H
#define _XHCI_MTK_POWER_H

struct xhci_hcor;

void enableXhciAllPortPower(struct xhci_hcor *hcor);
void enableAllClockPower(void);
void disablePortClockPower(void);

#endif
