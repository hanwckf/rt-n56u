#ifndef __RA_ESW_BASE_H__
#define __RA_ESW_BASE_H__

#include <linux/irqreturn.h>

u32 esw_reg_get(u32 addr);
void esw_reg_set(u32 addr, u32 data);

u32 esw_get_port_mib_tgoc(u32 port_id, u32 *HighPart);
u32 esw_get_port_mib_rgoc(u32 port_id, u32 *HighPart);
u32 esw_get_port_mib_tgpc(u32 port_id);
u32 esw_get_port_mib_rgpc(u32 port_id);
u32 esw_get_port_mib_tboc(u32 port_id);
u32 esw_get_port_mib_rboc(u32 port_id);
u32 esw_get_port_mib_tbpc(u32 port_id);
u32 esw_get_port_mib_rbpc(u32 port_id);
u32 esw_get_port_mib_tepc(u32 port_id);
u32 esw_get_port_mib_repc(u32 port_id);
u32 esw_get_port_mib_rfpc(u32 port_id);

void esw_mib_init(void);
void esw_irq_init(void);
void esw_irq_enable(void);
void esw_irq_disable(void);
void esw_irq_cancel_wq(void);
irqreturn_t esw_interrupt(int irq, void *dev_id);

extern void (*esw_link_status_hook)(u32 port_id, int port_link);

#endif

