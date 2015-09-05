#ifndef __RA_ETH_MT7621_H__
#define __RA_ETH_MT7621_H__

void ge2_int2_schedule_wq(void);
void ge2_int2_cancel_wq(void);

void mt7621_apll_trgmii_enable(void);

void mt7621_eth_init(void);

#endif

