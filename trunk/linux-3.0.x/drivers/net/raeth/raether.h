#ifndef __RAETHER_H__
#define __RAETHER_H__

#define RAETH_VERSION		"v3.0.6"
#define RAETH_DEV_NAME		"raeth"

//#define RAETH_DEBUG

#if defined (MEMORY_OPTIMIZATION)
#define NUM_RX_DESC		128
#define NUM_TX_DESC		128
#define NUM_RX_MAX_PROCESS	32
#else
#if defined (CONFIG_RALINK_RT2880) || defined (CONFIG_RALINK_RT3052)
#define NUM_RX_DESC		128
#define NUM_TX_DESC		128
#else
#define NUM_RX_DESC		256
#define NUM_TX_DESC		256
#endif
#define NUM_RX_MAX_PROCESS	16
#endif

#define DEV_NAME		"eth2"
#define DEV2_NAME		"eth3"

#define GMAC2_OFFSET		0x22
#if !defined (CONFIG_RALINK_RT6855A)
#define GMAC0_OFFSET		0x28
#else
#define GMAC0_OFFSET		0xE000
#endif

#define PSE_PORT_CPU		0
#define PSE_PORT_GMAC1		1
#define PSE_PORT_GMAC2		2
#if defined (CONFIG_RALINK_MT7621)
#define PSE_PORT_PPE		4
#else
#define PSE_PORT_PPE		6
#endif

#ifdef RAETH_DEBUG
#define RAETH_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define RAETH_PRINT(fmt, args...) { }
#endif

#endif
