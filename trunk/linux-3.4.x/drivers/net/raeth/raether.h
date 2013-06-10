#ifndef RA2882ETHEND_H
#define RA2882ETHEND_H

#define RAETH_VERSION	"v3.0.5"

#ifdef MEMORY_OPTIMIZATION
#define NUM_RX_DESC	128
#define NUM_TX_DESC	128
#define NUM_RX_MAX_PROCESS 64
#else
#define NUM_RX_DESC	256
#define NUM_TX_DESC	256
#define NUM_RX_MAX_PROCESS 16
#endif

#define DEV_NAME	"eth2"
#define DEV2_NAME	"eth3"

#define GMAC2_OFFSET	0x22
#define GMAC0_OFFSET	0x28

#define IRQ_ENET0	3

#define PSE_PORT_CPU	0
#define PSE_PORT_GMAC1	1
#define PSE_PORT_GMAC2	2
#define PSE_PORT_PPE	6

//#define RAETH_DEBUG
#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data);
u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data);

#endif
