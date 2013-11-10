#ifndef __RA_MAC_H__
#define __RA_MAC_H__

void enable_auto_negotiate(int ge);

void ra_mac1_addr_set(unsigned char p[6]);
#ifdef CONFIG_PSEUDO_SUPPORT
void ra_mac2_addr_set(unsigned char p[6]);
#endif

int  debug_proc_init(void);
void debug_proc_exit(void);

#endif
