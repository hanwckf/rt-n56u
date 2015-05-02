#ifndef __RA_PHY_H__
#define __RA_PHY_H__

void fe_phy_init_early(void);
void fe_phy_init(void);

void init_giga_phy(int ge);
void enable_autopoll_phy(int ge);
void ge1_set_mode(int ge_mode, int need_mdio);
void ge2_set_mode(int ge_mode, int need_mdio);

#endif
