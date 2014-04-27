/*

*/


#include "rt_config.h"


INT get_pkt_phymode_by_rxwi(RXWI_STRUC *rxwi)
{
	return rxwi->RXWI_O.phy_mode;
}

INT get_pkt_rssi_by_rxwi(RXWI_STRUC *rxwi, INT size, CHAR *rssi)
{
	switch (size) {
		case 3:
			rssi[2] = rxwi->RxWIRSSI2;
		case 2:
			rssi[1] = rxwi->RxWIRSSI1;
		case 1:
		default:
			rssi[0] = rxwi->RxWIRSSI0;
			break;
	}

	return 0;
}


INT get_pkt_snr_by_rxwi(RXWI_STRUC *rxwi, INT size, UCHAR *snr)
{
	switch (size) {
		case 3:
			snr[2] = rxwi->RxWISNR2;
		case 2:
			snr[1] = rxwi->RxWISNR1;
		case 1:
		default:
			snr[0] = rxwi->RxWISNR0;
			break;
	}
	
	return 0;
}

