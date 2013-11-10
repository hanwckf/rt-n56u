#ifndef __RA_PHY_H__
#define __RA_PHY_H__

void fe_phy_init(void);

#if defined (CONFIG_RAETH_ESW) && defined (CONFIG_RAETH_ESW_DOWN_PORTS)
void rt_esw_ports_down(void);
#endif

#if defined (CONFIG_RAETH_ESW) && defined (CONFIG_RAETH_ESW_DHCP_TOUCH)
void kill_sig_workq(struct work_struct *work);
#endif

#if defined (CONFIG_RALINK_RT6855) || defined(CONFIG_RALINK_RT6855A) || \
    defined (CONFIG_RALINK_MT7620)
//void esw_link_status_changed(int port_no, void *dev_id);
#endif

#if defined (CONFIG_RAETH_ESW)
irqreturn_t esw_interrupt(int irq, void *dev_id);
#endif


#endif