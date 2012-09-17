#ifndef RA_MAC_H
#define RA_MAC_H

void ra2880stop(END_DEVICE *ei_local);
void ra2880MacAddressSet(unsigned char p[6]);
void ra2880Mac2AddressSet(unsigned char p[6]);
void ethtool_init(struct net_device *dev);

void ra2880EnableInterrupt(void);

void dump_qos(void);
void dump_reg(void);
void dump_cp0(void);

int debug_proc_init(void);
void debug_proc_exit(void);

void enable_auto_negotiate(int ge);
void rt2880_gmac_hard_reset(void);

#endif
