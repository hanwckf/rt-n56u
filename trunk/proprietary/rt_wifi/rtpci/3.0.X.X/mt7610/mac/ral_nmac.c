/*

*/


#include "rt_config.h"


INT get_pkt_phymode_by_rxwi(RXWI_STRUC *rxwi)
{
	return rxwi->RXWI_N.phy_mode;
}

INT get_pkt_rssi_by_rxwi(RXWI_STRUC *rxwi, INT size, CHAR *rssi)
{
	if (size < (sizeof(rxwi->RXWI_N.rssi) / sizeof(UINT8)))
		NdisMoveMemory(rssi, &rxwi->RXWI_N.rssi[0], size);

	return 0;
}


INT get_pkt_snr_by_rxwi(RXWI_STRUC *rxwi, INT size, UCHAR *snr)
{
	// TODO: shiang-6590, fix me for SNR info of RXWI!!
	if (size < 3)
		NdisMoveMemory(snr, &rxwi->RXWI_N.bbp_rxinfo[0], size);

	return 0;
}

