#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>

#include <linux/timer.h>
#include <linux/init.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <asm/uaccess.h>

#define R1000_VERSION "1.03"
#define RELEASE_DATE "2006/05/26"
#define MODULENAME "r1000"
#define R1000_DRIVER_NAME MODULENAME R1000_VERSION ", the Linux device driver for Realtek Ethernet Controllers"
#define PFX MODULENAME ": "


#undef R1000_DEBUG
#undef R1000_JUMBO_FRAME_SUPPORT
//#undef	R1000_HW_FLOW_CONTROL_SUPPORT
#define	R1000_HW_FLOW_CONTROL_SUPPORT


#undef R1000_IOCTL_SUPPORT
#undef R1000_DYNAMIC_CONTROL
#define R1000_USE_IO
//#undef R1000_USE_IO


#ifdef R1000_DEBUG
	#define assert(expr) \
        	if(!(expr)) { printk( "Assertion failed! %s,%s,%s,line=%d\n", #expr,__FILE__,__FUNCTION__,__LINE__); }
	#define DBG_PRINT( fmt, args...)   printk("r1000: " fmt, ## args);
#else
	#define assert(expr) do {} while (0)
	#define DBG_PRINT( fmt, args...)   ;
#endif	// end of #ifdef R1000_DEBUG

/* media options */
#define MAX_UNITS 8

/* MAC address length*/
#define MAC_ADDR_LEN        6

#define RX_FIFO_THRESH      7       /* 7 means NO threshold, Rx buffer level before first PCI xfer.  */
#define RX_DMA_BURST        7       /* Maximum PCI burst, '6' is 1024 */
#define TX_DMA_BURST        7       /* Maximum PCI burst, '6' is 1024 */
#define ETTh                0x3F    /* 0x3F means NO threshold */

#define ETH_HDR_LEN         14
#define DEFAULT_MTU         1500
#define DEFAULT_RX_BUF_LEN  1536


#ifdef R1000_JUMBO_FRAME_SUPPORT
#define MAX_JUMBO_FRAME_MTU	( 10000 )
#define MAX_RX_SKBDATA_SIZE	( MAX_JUMBO_FRAME_MTU + ETH_HDR_LEN )
#else
//#define MAX_RX_SKBDATA_SIZE 1600
#define MAX_RX_SKBDATA_SIZE 1608
#endif //end #ifdef R1000_JUMBO_FRAME_SUPPORT


#define InterFrameGap       0x03    /* 3 means InterFrameGap = the shortest one */

#define NUM_TX_DESC         1024     /* Number of Tx descriptor registers*/
#define NUM_RX_DESC         1024     /* Number of Rx descriptor registers*/

#define RTL_MIN_IO_SIZE     0x80
#define TX_TIMEOUT          (6*HZ)
#define R1000_TIMER_EXPIRE_TIME 100 //100

#ifdef R1000_USE_IO
#define RTL_W8(reg, val8)   outb ((val8), ioaddr + (reg))
#define RTL_W16(reg, val16) outw ((val16), ioaddr + (reg))
#define RTL_W32(reg, val32) outl ((val32), ioaddr + (reg))
#define RTL_R8(reg)         inb (ioaddr + (reg))
#define RTL_R16(reg)        inw (ioaddr + (reg))
#define RTL_R32(reg)        ((unsigned long) inl (ioaddr + (reg)))
#else	//R1000_USE_IO
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,6)
/* write/read MMIO register  for Linux kernel 2.4.x*/
#define RTL_W8(reg, val8)   writeb ((val8), ioaddr + (reg))
#define RTL_W16(reg, val16) writew ((val16), ioaddr + (reg))
#define RTL_W32(reg, val32) writel ((val32), ioaddr + (reg))
#define RTL_R8(reg)         readb (ioaddr + (reg))
#define RTL_R16(reg)        readw (ioaddr + (reg))
#define RTL_R32(reg)        ((unsigned long) readl (ioaddr + (reg)))
#else
/* write/read MMIO register  for Linux kernel 2.6.x*/
#define RTL_W8(reg, val8)   iowrite8 ((val8), (void *)(ioaddr + (reg)))
#define RTL_W16(reg, val16) iowrite16 ((val16), (void *)(ioaddr + (reg)))
#define RTL_W32(reg, val32) iowrite32 ((val32), (void *)(ioaddr + (reg)))
#define RTL_R8(reg)         ioread8 ((void *)(ioaddr + (reg)))
#define RTL_R16(reg)        ioread16 ((void *)(ioaddr + (reg)))
#define RTL_R32(reg)        ((unsigned long) ioread32 ((void *)(ioaddr + (reg))))
#endif
#endif	//R1000_USE_IO

#define MCFG_METHOD_1		0x01
#define MCFG_METHOD_2		0x02
#define MCFG_METHOD_3		0x03
#define MCFG_METHOD_4		0x04
#define MCFG_METHOD_5		0x05
#define MCFG_METHOD_11		0x0B
#define MCFG_METHOD_12		0x0C
#define MCFG_METHOD_13		0x0D
#define MCFG_METHOD_14		0x0E
#define MCFG_METHOD_15		0x0F

#define PCFG_METHOD_1		0x01	//PHY Reg 0x03 bit0-3 == 0x0000
#define PCFG_METHOD_2		0x02	//PHY Reg 0x03 bit0-3 == 0x0001
#define PCFG_METHOD_3		0x03	//PHY Reg 0x03 bit0-3 == 0x0002


#ifdef R1000_DYNAMIC_CONTROL
#include "r1000_callback.h"
#endif  //R1000_DYNAMIC_CONTROL

enum r1000_registers {
	MAC0 = 0x0,
	MAR0 = 0x8,
	TxDescStartAddr	= 0x20,
	TxHDescStartAddr= 0x28,
	FLASH	= 0x30,
	ERSR	= 0x36,
	ChipCmd	= 0x37,
	TxPoll	= 0x38,
	IntrMask = 0x3C,
	IntrStatus = 0x3E,
	TxConfig = 0x40,
	RxConfig = 0x44,
	RxMissed = 0x4C,
	Cfg9346 = 0x50,
	Config0	= 0x51,
	Config1	= 0x52,
	Config2	= 0x53,
	Config3	= 0x54,
	Config4	= 0x55,
	Config5	= 0x56,
	MultiIntr = 0x5C,
	PHYAR	= 0x60,
	TBICSR	= 0x64,
	TBI_ANAR = 0x68,
	TBI_LPAR = 0x6A,
	PHYstatus = 0x6C,
	Off7Ch = 0x7C,
	RxMaxSize = 0xDA,
	CPlusCmd = 0xE0,
	RxDescStartAddr	= 0xE4,
	ETThReg	= 0xEC,
	FuncEvent	= 0xF0,
	FuncEventMask	= 0xF4,
	FuncPresetState	= 0xF8,
	FuncForceEvent	= 0xFC,
};

enum r1000_register_content {
	/*InterruptStatusBits*/
	SYSErr 		= 0x8000,
	PCSTimeout	= 0x4000,
	SWInt		= 0x0100,
	TxDescUnavail	= 0x80,
	RxFIFOOver 	= 0x40,
	LinkChg 	= 0x20,
	RxOverflow 	= 0x10,
	TxErr 	= 0x08,
	TxOK 	= 0x04,
	RxErr 	= 0x02,
	RxOK 	= 0x01,

	/*RxStatusDesc*/
	RxRES = 0x00200000,
	RxCRC = 0x00080000,
	RxRUNT= 0x00100000,
	RxRWT = 0x00400000,

	/*ChipCmdBits*/
	CmdReset = 0x10,
	CmdRxEnb = 0x08,
	CmdTxEnb = 0x04,
	RxBufEmpty = 0x01,

	/*Cfg9346Bits*/
	Cfg9346_Lock = 0x00,
	Cfg9346_Unlock = 0xC0,

	/*rx_mode_bits*/
	AcceptErr = 0x20,
	AcceptRunt = 0x10,
	AcceptBroadcast = 0x08,
	AcceptMulticast = 0x04,
	AcceptMyPhys = 0x02,
	AcceptAllPhys = 0x01,

	/*RxConfigBits*/
	RxCfgFIFOShift = 13,
	RxCfgDMAShift = 8,

	/*TxConfigBits*/
	TxInterFrameGapShift = 24,
	TxDMAShift = 8,

	/*rtl8169_PHYstatus (MAC offset 0x6C)*/
	TBI_Enable	= 0x80,
	TxFlowCtrl	= 0x40,
	RxFlowCtrl	= 0x20,
	_1000Mbps	= 0x10,
	_100Mbps	= 0x08,
	_10Mbps		= 0x04,
	LinkStatus	= 0x02,
	FullDup		= 0x01,

	/*GIGABIT_PHY_registers*/
	PHY_CTRL_REG = 0,
	PHY_STAT_REG = 1,
	PHY_AUTO_NEGO_REG = 4,
	PHY_1000_CTRL_REG = 9,

	/*GIGABIT_PHY_REG_BIT*/
	PHY_Restart_Auto_Nego	= 0x0200,
	PHY_Enable_Auto_Nego	= 0x1000,

	//PHY_STAT_REG = 1;
	PHY_Auto_Neco_Comp	= 0x0020,

	//PHY_AUTO_NEGO_REG = 4;
	PHY_Cap_10_Half		= 0x0020,
	PHY_Cap_10_Full		= 0x0040,
	PHY_Cap_100_Half	= 0x0080,
	PHY_Cap_100_Full	= 0x0100,

	//PHY_1000_CTRL_REG = 9;
	PHY_Cap_1000_Full	= 0x0200,
	PHY_Cap_1000_Half	= 0x0100,

	PHY_Cap_PAUSE		= 0x0400,
	PHY_Cap_ASYM_PAUSE	= 0x0800,

	PHY_Cap_Null		= 0x0,

	/*_MediaType*/
	_10_Half	= 0x01,
	_10_Full	= 0x02,
	_100_Half	= 0x04,
	_100_Full	= 0x08,
	_1000_Full	= 0x10,

	/*_TBICSRBit*/
	TBILinkOK 	= 0x02000000,
};



enum _DescStatusBit {
	OWNbit	= 0x80000000,
	EORbit	= 0x40000000,
	FSbit	= 0x20000000,
	LSbit	= 0x10000000,
};


struct TxDesc {
	u32		status;
	u32		vlan_tag;
	u32		buf_addr;
	u32		buf_Haddr;
};

struct RxDesc {
	u32		status;
	u32		vlan_tag;
	u32		buf_addr;
	u32		buf_Haddr;
};

#define r1000_request_timer( timer, timer_expires, timer_func, timer_data ) \
{ \
	init_timer(timer); \
	timer->expires = (unsigned long)(jiffies + timer_expires); \
	timer->data = (unsigned long)(timer_data); \
	timer->function = (void *)(timer_func); \
	add_timer(timer); \
	DBG_PRINT("request_timer at 0x%08lx\n", (unsigned long)timer); \
}

#define r1000_delete_timer( del_timer_t ) \
{ \
	del_timer(del_timer_t); \
	DBG_PRINT("delete_timer at 0x%08lx\n", (unsigned long)del_timer_t); \
}

#define r1000_mod_timer( timer, timer_expires ) \
{ \
	mod_timer( timer, jiffies + timer_expires ); \
}

typedef struct timer_list rt_timer_t;

struct r1000_private {
	unsigned long ioaddr;                /* memory map physical address*/
	struct pci_dev *pci_dev;                /* Index of PCI device  */
	struct net_device_stats stats;          /* statistics of net device */
	spinlock_t lock;                        /* spin lock flag */
	int chipset;
	int mcfg;
	int pcfg;
	rt_timer_t r1000_timer;
	unsigned long expire_time;

	unsigned long phy_link_down_cnt;
	unsigned long cur_rx;                   /* Index into the Rx descriptor buffer of next Rx pkt. */
	unsigned long cur_tx;                   /* Index into the Tx descriptor buffer of next Rx pkt. */
	unsigned long dirty_tx;
	struct	TxDesc	*TxDescArray;           /* Index of 256-alignment Tx Descriptor buffer */
	struct	RxDesc	*RxDescArray;           /* Index of 256-alignment Rx Descriptor buffer */
	struct	sk_buff	*Tx_skbuff[NUM_TX_DESC];/* Index of Transmit data buffer */
	struct	sk_buff	*Rx_skbuff[NUM_RX_DESC];/* Receive data buffer */
	unsigned char   drvinit_fail;

	dma_addr_t txdesc_array_dma_addr[NUM_TX_DESC];
	dma_addr_t rxdesc_array_dma_addr[NUM_RX_DESC];
	dma_addr_t rx_skbuff_dma_addr[NUM_RX_DESC];

	void *txdesc_space;
	dma_addr_t txdesc_phy_dma_addr;
	int sizeof_txdesc_space;

	void *rxdesc_space;
	dma_addr_t rxdesc_phy_dma_addr;
	int sizeof_rxdesc_space;

	int curr_mtu_size;
	int tx_pkt_len;
	int rx_pkt_len;

	int hw_rx_pkt_len;

#ifdef R1000_DYNAMIC_CONTROL
	struct r1000_cb_t rt;
#endif //end #ifdef R1000_DYNAMIC_CONTROL

	unsigned char   linkstatus;
};


