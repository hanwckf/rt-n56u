#ifndef __RA_ESW_RT305X_H__
#define __RA_ESW_RT305X_H__

void rt305x_esw_init(void);

int  rt305x_esw_wait_wt_mac(void);
int  rt305x_esw_mac_table_clear(int static_only);

void mt7628_esw_eee_enable(int is_eee_enabled);

#endif
