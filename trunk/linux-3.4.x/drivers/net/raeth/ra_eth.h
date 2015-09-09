#ifndef __RA_ETH_H__
#define __RA_ETH_H__

struct net_device;
struct rtnl_link_stats64;

void fe_eth_reset(void);
void fe_esw_init(void);
void fe_pse_init(void);
void fe_gdm_init(struct net_device *dev);
void fe_cdm_init(struct net_device *dev);
void fe_cdm_update_vlan_tx(const u16 *vlan_id_map);

void fe_dma_start(void);
void fe_dma_stop(void);

void fe_irq_enable(void);
void fe_irq_disable(void);

void fe_gdm1_set_mac(const u8 *mac);
void fe_gdm2_set_mac(const u8 *mac);

void fe_gdm1_fetch_mib(struct rtnl_link_stats64 *stat);
void fe_gdm2_fetch_mib(struct rtnl_link_stats64 *stat);

void ge1_set_mode(int ge_mode, int need_mdio);
void ge2_set_mode(int ge_mode, int need_mdio);

#endif
