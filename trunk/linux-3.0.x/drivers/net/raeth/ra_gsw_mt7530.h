#ifndef __RA_GSW_MT7530_H__
#define __RA_GSW_MT7530_H__

void mt7530_gsw_init(void);
void mt7530_gsw_set_smac(const u8 *mac);
void mt7530_gsw_set_csr_delay(int is_link_100);

void mt7530_gsw_eee_enable(int is_eee_enabled);
void mt7530_gsw_eee_on_link(u32 port_id, int port_link, int is_eee_enabled);

#endif
