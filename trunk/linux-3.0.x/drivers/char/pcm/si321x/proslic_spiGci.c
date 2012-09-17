#define LPT

#ifdef LPT
#include "proslic_api/example_drivers/win/proslic_ppt_mb/si321x/proslic_spiGci_parallelPort.c"
#else
	#ifdef USB
	#include "proslic_api/example_drivers/win/usb/proslic_spiGci_usb.c"
	#endif
#endif