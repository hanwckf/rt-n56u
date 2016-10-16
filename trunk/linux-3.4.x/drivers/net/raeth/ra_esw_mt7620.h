#ifndef __RA_ESW_MT7620_H__
#define __RA_ESW_MT7620_H__

void mt7620_esw_init(void);

void mt7620_esw_pvid_set(u32 port_id, u32 pvid, u32 prio);
int  mt7620_esw_vlan_set_idx(u32 idx, u32 cvid, u32 port_member);
int  mt7620_esw_vlan_clear_idx(u32 idx);

int  mt7620_esw_wait_wt_mac(void);
int  mt7620_esw_mac_table_clear(int static_only);

void mt7620_esw_eee_enable(int is_eee_enabled);
void mt7620_esw_eee_on_link(u32 port_id, int port_link, int is_eee_enabled);

#endif
