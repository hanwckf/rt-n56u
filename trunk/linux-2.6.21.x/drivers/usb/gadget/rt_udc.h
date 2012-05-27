/*
 *	Copyright (C) 2009 Y.Y. Huang, Ralink Tech.(yy_huang@ralinktech.com)
 *
 *	This udc driver is now under testing and code is based on pxa2xx_udc.h
 *	Please use it with your own risk!
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 */

#ifndef __LINUX_USB_GADGET_RT_UDC_H
#define __LINUX_USB_GADGET_RT_UDC_H

#include <linux/types.h>
#include <linux/config.h>

#if defined (CONFIG_RALINK_RT3883) || defined (CONFIG_RALINK_RT3352)
#define IN_EP_NUM		2
#define OUT_EP_NUM		2
#elif defined (CONFIG_RALINK_RT5350)
#define IN_EP_NUM		1
#define OUT_EP_NUM		1
#else
#error "Please define a platform."
#endif

/* Helper macros */
#define EP_IDX(ep)		((ep->bEndpointAddress & ~USB_DIR_IN)+(EP_DIR(ep)? 0:IN_EP_NUM)) /* IN:1, OUT:0 */
#define EP_NO(ep)		((ep->bEndpointAddress & ~USB_DIR_IN)) 							/* IN:1, OUT:0 */
#define EP_DIR(ep)		((ep->bEndpointAddress) & USB_DIR_IN ? 1 : 0)
#define EP_IN			1
#define EP_OUT			0
#define RT_USB_NB_EP	(IN_EP_NUM + OUT_EP_NUM + 1)

/* Driver structures */
struct rt_request {
	struct usb_request			req;
	struct list_head			queue;
	unsigned int				in_use;
	struct rt_ep_struct			*rt_ep;			// test for rx tasklet
	int							zlp_dma_done;	// used for DMA ZLP packet.
};

enum ep0_state {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_NO_DATA_PHASE,
	EP0_STALL,
};

struct rt_ep_struct {
	struct usb_ep				ep;
	struct rt_udc_struct		*rt_usb;
	struct list_head			queue;
	unsigned char				stopped;
	unsigned char				bEndpointAddress;
	unsigned char				bmAttributes;

	unsigned char				pending;
	unsigned int				rx_done_count;	/* used by OUT EP only */
	unsigned int				tx_done_count;	/* used by OUT EP only */
};

struct rt_udc_struct {
	struct usb_gadget			gadget;
	struct usb_gadget_driver	*driver;
	struct device				*dev;
	struct rt_ep_struct			rt_ep[RT_USB_NB_EP];
	/* struct clk				*clk; */
	struct timer_list			timer;
	enum ep0_state				ep0state;
	struct resource				*res;
	void __iomem				*base;
	unsigned char				set_config;
	int					cfg,
						intf,
						alt,
						interrupt;
};

#define USB_BASE		(0xB0120000)

#define OUT0BC			(0x000)
#define IN0BC			(0x001)
#define EP0CS			(0x002)

#define OUT1CON			(0x00A)
#define IN1CON			(0x00E)
#define OUT2CON			(0x012)
#define IN2CON			(0x016)
#define OUT3CON			(0x01A)
#define IN3CON			(0x01E)
#define OUT4CON			(0x022)
#define IN4CON			(0x026)


#define EP0INDAT		(0x100)
#define EP0OUTDAT		(0x140)
#define SETUPDATA		(0x180)

#define IN07IRQ			(0x188)
#define IN815IRQ		(0x189)
#define OUT07IRQ		(0x18A)
#define OUT815IRQ		(0x18B)
#define USBIRQ			(0x18C)
#define OUT07PNGIRQ		(0x18E)
#define OUT815PNGIRQ	(0x18F)

#define IN07IEN			(0x194)
#define OUT07IEN		(0x196)
#define USBIEN			(0x198)

#define OUT07PNGIEN		(0x19A)
#define OUT815PNGIEN	(0x19B)

#define ENDPRST			(0x1A2)
#define ENDPRST_IO		(0x1 << 4)
#define ENDPRST_TOGRST	(0x1 << 5)
#define ENDPRST_FIFORST	(0x1 << 6)

#define USBCS                   (0x1A3)
#define USBCS_DISCON            (0x1 << 6)

#define FIFOCTRL		(0x1A8)

#define EP_CS_EP0_STALL		(0x1 << 0)
#define EP_CS_EP0_HSNAK		(0x1 << 1)
#define EP_CS_EP0_INBSY		(0x1 << 2)
#define EP_CS_EP0_OUTBSY	(0x1 << 3)
#define EP_CS_AUTO		(0x1 << 4)
#define EP_CS_NPAK1		(0x1 << 3)
#define EP_CS_NPAK0		(0x1 << 2)
#define EP_CS_BSY		(0x1 << 1)
#define EP_CS_ERR		(0x1 << 0)

#define EP0_OUT_BSY		(0x1 << 3)
#define EP0_IN_BSY		(0x1 << 2)

#define USB_INTR_HSPEED				(0x20)
#define USB_INTR_RESET				(0x10)
#define USB_INTR_SUSPEND			(0x08)
#define USB_INTR_SETUP_TOKEN		(0x04)
#define USB_INTR_SOF				(0x02)
#define USB_INTR_SETUP_TOKEN_VALID	(0x01)

/* UDMA */
#define RTUSB_UDMA_CTRL				(USB_BASE + 0x800)
#define RTUSB_UDMA_WRR				(USB_BASE + 0x804)

/* PDMA */
#define RTUSB_TX_BASE_PTR0        (USB_BASE + 0x1000)
#define RTUSB_TX_MAX_CNT0         (USB_BASE + 0x1004)
#define RTUSB_TX_CTX_IDX0         (USB_BASE + 0x1008)
#define RTUSB_TX_DTX_IDX0         (USB_BASE + 0x100C)
#define RTUSB_TX_BASE_PTR1        (USB_BASE + 0x1010)
#define RTUSB_TX_MAX_CNT1         (USB_BASE + 0x1014)
#define RTUSB_TX_CTX_IDX1         (USB_BASE + 0x1018)
#define RTUSB_TX_DTX_IDX1         (USB_BASE + 0x101C)
#define RTUSB_RX_BASE_PTR0        (USB_BASE + 0x1100)
#define RTUSB_RX_MAX_CNT0         (USB_BASE + 0x1104)
#define RTUSB_RX_CALC_IDX0        (USB_BASE + 0x1108)
#define RTUSB_RX_DRX_IDX0         (USB_BASE + 0x110C)
#define RTUSB_PDMA_GLO_CFG        (USB_BASE + 0x1204)

#define RTUSB_TX_WB_DDONE			(0x1 << 6)
#define RTUSB_RX_DMA_BUSY			(0x1 << 3)
#define RTUSB_RX_DMA_EN				(0x1 << 2)
#define RTUSB_TX_DMA_BUSY			(0x1 << 1)
#define RTUSB_TX_DMA_EN				(0x1 << 0)

#define RTUSB_PDMA_RST_IDX        (USB_BASE + 0x1208)

#define RTUSB_RST_DRX_IDX1			(0x1 << 17)
#define RTUSB_RST_DRX_IDX0			(0x1 << 16)
#define RTUSB_RST_DTX_IDX3			(0x1 << 3)
#define RTUSB_RST_DTX_IDX2			(0x1 << 2)
#define RTUSB_RST_DTX_IDX1			(0x1 << 1)
#define RTUSB_RST_DTX_IDX0			(0x1 << 0)

#define RTUSB_DELAY_INT_CFG       (USB_BASE + 0x120C)
#define RTUSB_INT_STATUS		  (USB_BASE + 0x1220)
#define RTUSB_RX_DONE_INT1			(0x1 << 17)
#define RTUSB_RX_DONE_INT0			(0x1 << 16)
#define RTUSB_TX_DONE_INT3			(0x1 << 3)
#define RTUSB_TX_DONE_INT2			(0x1 << 2)
#define RTUSB_TX_DONE_INT1			(0x1 << 1)
#define RTUSB_TX_DONE_INT0			(0x1 << 0)

#define RTUSB_INT_MASK			  (USB_BASE + 0x1228)
#define RTUSB_RX_DONE_INT_MSK1		(0x1 << 17)
#define RTUSB_RX_DONE_INT_MSK0		(0x1 << 16)
#define RTUSB_TX_DONE_INT_MSK3		(0x1 << 3)
#define RTUSB_TX_DONE_INT_MSK2		(0x1 << 2)
#define RTUSB_TX_DONE_INT_MSK1		(0x1 << 1)
#define RTUSB_TX_DONE_INT_MSK0		(0x1 << 0)


/*=========================================
      PDMA RX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO1_  PDMA_RXD_INFO1_T;

struct _PDMA_RXD_INFO1_ {
    unsigned int    PDP0;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO2_    PDMA_RXD_INFO2_T;

struct _PDMA_RXD_INFO2_ {
    unsigned int    PLEN1                 : 14;
    unsigned int    LS1                   : 1;
    unsigned int    UN_USED               : 1;
    unsigned int    PLEN0                 : 14;
    unsigned int    LS0                   : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO3_  PDMA_RXD_INFO3_T;

struct _PDMA_RXD_INFO3_ {
    unsigned int    PDP1;
};
//-------------------------------------------------
typedef struct _PDMA_RXD_INFO4_    PDMA_RXD_INFO4_T;

struct _PDMA_RXD_INFO4_ {
    unsigned int        Rx_bcnt:16;
    unsigned int        Reserved1:8;
    unsigned int        Out_ep_addr:4;
    unsigned int        Reserved0:4;
};
struct PDMA_rxdesc {
    PDMA_RXD_INFO1_T rxd_info1;
    PDMA_RXD_INFO2_T rxd_info2;
    PDMA_RXD_INFO3_T rxd_info3;
    PDMA_RXD_INFO4_T rxd_info4;
};
/*=========================================
      PDMA TX Descriptor Format define
=========================================*/
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO1_  PDMA_TXD_INFO1_T;

struct _PDMA_TXD_INFO1_ {
    unsigned int    SDP0;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO2_    PDMA_TXD_INFO2_T;

struct _PDMA_TXD_INFO2_ {
    unsigned int    SDL1                  : 14;
    unsigned int    LS1_bit               : 1;
    unsigned int    BURST_bit             : 1;
    unsigned int    SDL0                  : 14;
    unsigned int    LS0_bit               : 1;
    unsigned int    DDONE_bit             : 1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO3_  PDMA_TXD_INFO3_T;

struct _PDMA_TXD_INFO3_ {
    unsigned int    SDP1;
};
//-------------------------------------------------
typedef struct _PDMA_TXD_INFO4_    PDMA_TXD_INFO4_T;
struct _PDMA_TXD_INFO4_ {
    unsigned int        rsv2:24;
    unsigned int        In_ep_addr:4;
    unsigned int        rsv:4;
};

struct PDMA_txdesc {
    PDMA_TXD_INFO1_T txd_info1;
    PDMA_TXD_INFO2_T txd_info2;
    PDMA_TXD_INFO3_T txd_info3;
    PDMA_TXD_INFO4_T txd_info4;
};


#ifdef DEBUG
#define DBG do{ if(debuglevel) printk("%s()\n", __FUNCTION__); }while(0);
#define DD	do{ printk("%s: %s %d\n", driver_name, __FUNCTION__, __LINE__); } while(0);
#define xprintk(fmt, args...)	do{ if(debuglevel) printk(fmt, ## args); } while(0);
#else
#define DBG
#define DD
#define xprintk(fmt, args...)
#endif

#define FATAL_ERROR(fmt, args...)	do{ printk(fmt, ## args); printk("\n###############  ERROR  #####################\n %s %d\n###############  ERROR  #####################\n",  __FUNCTION__, __LINE__);  BUG(); } while(0)

static void inline dump_usbirq(u32 irqreg)
{
	if(irqreg)
		xprintk("U%s%s%s%s%s%s\n", 
			(irqreg & USB_INTR_SOF) ? "sof" : "",
			(irqreg & USB_INTR_RESET) ? " rst" : "",
			(irqreg & USB_INTR_SUSPEND) ? " sus" : "",
			(irqreg & USB_INTR_SETUP_TOKEN) ? "st" : "",
			(irqreg & USB_INTR_SETUP_TOKEN_VALID) ? "sv" : "",
			(irqreg & USB_INTR_HSPEED) ? " HS" : "");

//  if(irqreg & USB_INTR_SETUP_TOKEN)
//  	printk("ST\n");
//  if(irqreg & USB_INTR_SETUP_TOKEN_VALID)
//  	printk("SV\n");

}

static void inline dump_epirq(u32 irqreg, u32 ienreg, int dir)
{
	if(irqreg)
	xprintk("%s%x\n", dir? "I" : "O", irqreg);
}

static __inline__ u32 usb_read(u32 addr)
{
	return ioread32( (void __iomem *)(USB_BASE + (addr << 2)) );
}

static __inline__ void usb_write(u32 addr, u32 value)
{
	iowrite32(value, (void __iomem *)(USB_BASE + (addr << 2)) );
}

static __inline__ void reg_write(u32 addr, u32 value)
{
	iowrite32(value, (void __iomem *)0x0 + addr);
}

static __inline__ u32 reg_read(u32 addr)
{
	return ioread32( (void __iomem *)0x0 + addr);
}


static void handle_pending_epoutirq(struct rt_udc_struct *rt_usb, struct rt_ep_struct *rt_ep, struct rt_request *req);

/* Debug macros */
#ifdef DEBUG
#define DEBUG_REQ
#define DEBUG_TRX
#define DEBUG_INIT
#define DEBUG_EP0
#define DEBUG_EPX
#define DEBUG_ERR

#ifdef DEBUG_REQ
	#define D_REQ(dev, args...)	printk(args)
#else
	#define D_REQ(dev, args...)	do {} while (0)
#endif /* DEBUG_REQ */

#ifdef DEBUG_TRX
	#define D_TRX(dev, args...)	printk(args)
#else
	#define D_TRX(dev, args...)	do {} while (0)
#endif /* DEBUG_TRX */

#ifdef DEBUG_INIT
	#define D_INI(dev, args...)	printk(args)
#else
	#define D_INI(dev, args...)	do {} while (0)
#endif /* DEBUG_INIT */

#ifdef DEBUG_EP0
	static const char *state_name[] = {
		"IDLE",
		"IN",
		"OUT",
		"NODATA",
		"STALL"
	};
	#define D_EP0(dev, args...) printk(args)
#else
	#define D_EP0(dev, args...)	do {} while (0)
#endif /* DEBUG_EP0 */

#ifdef DEBUG_EPX
	#define D_EPX(dev, args...)	printk(args)
#else
	#define D_EPX(dev, args...)	do {} while (0)
#endif /* DEBUG_EP0 */

#ifdef DEBUG_ERR
	#define D_ERR(dev, args...)	printk(args)
#else
	#define D_ERR(dev, args...)	do {} while (0)
#endif

#else
	#define D_REQ(dev, args...)		do {} while (0)
	#define D_TRX(dev, args...)		do {} while (0)
	#define D_INI(dev, args...)		do {} while (0)
	#define D_EP0(dev, args...)		do {} while (0)
	#define D_EPX(dev, args...)		do {} while (0)
	#define dump_ep_intr(x, y, z, i)	do {} while (0)
	#define dump_intr(x, y, z)		do {} while (0)
	#define dump_ep_stat(x, y)		do {} while (0)
	#define dump_usb_stat(x, y)		do {} while (0)
	#define dump_req(x, y, z)		do {} while (0)
	#define D_ERR(dev, args...)		do {} while (0)
#endif /* DEBUG */

#endif /* __LINUX_USB_GADGET_RT_UDC_H */
