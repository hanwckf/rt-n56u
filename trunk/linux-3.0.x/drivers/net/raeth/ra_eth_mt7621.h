#ifndef __RA_ETH_MT7621_H__
#define __RA_ETH_MT7621_H__

void ge2_int2_schedule_wq(void);
void ge2_int2_cancel_wq(void);

void mt7621_apll_trgmii_enable(void);

void mt7621_eth_init(void);

void mt7621_eth_gdma_vlan_pvid(int ge2, u32 vlan_id, u32 prio);
void mt7621_eth_gdma_vlan_insv(int ge2, int insv_en);
void mt7621_eth_gdma_vlan_untag(int ge2, int untag_en);

#endif

