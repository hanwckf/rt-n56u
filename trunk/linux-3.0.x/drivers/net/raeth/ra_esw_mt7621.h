#ifndef __RA_ESW_MT7621_H__
#define __RA_ESW_MT7621_H__

void mt7621_esw_init(void);
void mt7621_esw_fc_delay_set(int is_link_100);

void mt7621_esw_eee_enable(int is_eee_enabled);
void mt7621_esw_eee_on_link(u32 port_id, int port_link, int is_eee_enabled);

#endif
