/* FIXME: Only this does work for u-boot... find out why... [RS] */
# undef	io_p2v
# undef __REG
# ifndef __ASSEMBLY__
#  define io_p2v(PhAdd)    (PhAdd)
#  define __REG(x)	(*((volatile u32 *)io_p2v(x)))
#  define __REG2(x,y)	(*(volatile u32 *)((u32)&__REG(x) + (y)))
# else
#  define __REG(x) (x)
# endif


/*
 * UART registers
 */
#if defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#define RT2880_UART1	0x0C00  /* UART Lite */
#define RT2880_UART2	0x0D00  /* UART Lite */
#define RT2880_UART3	0x0E00  /* UART Lite */
#define CFG_RT2880_CONSOLE	RT2880_UART1
#else
#define RT2880_UART1	0x0500
#define RT2880_UART2	0x0C00  /* UART Lite */
#define CFG_RT2880_CONSOLE	RT2880_UART2
#endif


#if defined (MT7621_FPGA_BOARD) || defined (MT7621_ASIC_BOARD) || defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#define RT2880_UART_RBR_OFFSET	0x00
#define RT2880_UART_TBR_OFFSET	0x00
#define RT2880_UART_IER_OFFSET	0x04
#define RT2880_UART_IIR_OFFSET	0x08
#define RT2880_UART_FCR_OFFSET	0x08
#define RT2880_UART_LCR_OFFSET	0x0C
#define RT2880_UART_MCR_OFFSET	0x10
#define RT2880_UART_LSR_OFFSET	0x14
#define RT2880_UART_DLL_OFFSET	0x00
#define RT2880_UART_DLM_OFFSET	0x04
#else
#define RT2880_UART_RBR_OFFSET	0x00
#define RT2880_UART_TBR_OFFSET	0x04
#define RT2880_UART_IER_OFFSET	0x08
#define RT2880_UART_IIR_OFFSET	0x0C
#define RT2880_UART_FCR_OFFSET	0x10
#define RT2880_UART_LCR_OFFSET	0x14
#define RT2880_UART_MCR_OFFSET	0x18
#define RT2880_UART_LSR_OFFSET	0x1C
#define RT2880_UART_DLL_OFFSET	0x2C
#define RT2880_UART_DLM_OFFSET	0x30
#endif

#define RBR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_RBR_OFFSET)
#define TBR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_TBR_OFFSET)
#define IER(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_IER_OFFSET)
#define IIR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_IIR_OFFSET)
#define FCR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_FCR_OFFSET)
#define LCR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_LCR_OFFSET)
#define MCR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_MCR_OFFSET)
#define LSR(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_LSR_OFFSET)
#define DLL(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_DLL_OFFSET)
#define DLM(x)		__REG(RALINK_SYSCTL_BASE+(x)+RT2880_UART_DLM_OFFSET)

#define IER_ELSI	(1 << 2)	/* Receiver Line Status Interrupt Enable */
#define IER_ETBEI	(1 << 1)	/* Transmit Buffer Empty Interrupt Enable */
#define IER_ERBFI	(1 << 0)	/* Data Ready or Character Time-Out Interrupt Enable */

#define IIR_FIFOES1	(1 << 7)	/* FIFO Mode Enable Status */
#define IIR_FIFOES0	(1 << 6)	/* FIFO Mode Enable Status */
#define IIR_IID3	(1 << 3)	/* Interrupt Source Encoded */
#define IIR_IID2	(1 << 2)	/* Interrupt Source Encoded */
#define IIR_IID1	(1 << 1)	/* Interrupt Source Encoded */
#define IIR_IP		(1 << 0)	/* Interrupt Pending (active low) */

#define FCR_RXTRIG1	(1 << 7)	/* Receiver Interrupt Trigger Level */
#define FCR_RXTRIG0	(1 << 6)	/* Receiver Interrupt Trigger Level */
#define FCR_TXTRIG1	(1 << 5)	/* Transmitter Interrupt Trigger Level */
#define FCR_TXTRIG0	(1 << 4)	/* Transmitter Interrupt Trigger Level */
#define FCR_DMAMODE	(1 << 3)	/* Enable DMA transfers */
#define FCR_TXRST	(1 << 2)	/* Reset Transmitter FIFO */
#define FCR_RXRST	(1 << 1)	/* Reset Receiver FIFO */
#define FCR_FIFOE	(1 << 0)	/* Transmit and Receive FIFO Enable */


#define LCR_DLAB	(1 << 7)	/* Divisor Latch Access Bit */
#define LCR_SB		(1 << 6)	/* Set Break */
#define LCR_STKYP	(1 << 5)	/* Sticky Parity */
#define LCR_EPS		(1 << 4)	/* Even Parity Select */
#define LCR_PEN		(1 << 3)	/* Parity Enable */
#define LCR_STB		(1 << 2)	/* Stop Bit */
#define LCR_WLS1	(1 << 1)	/* Word Length Select */
#define LCR_WLS0	(1 << 0)	/* Word Length Select */

#define MCR_LOOP	(1 << 4)	/* Loop-back Mode Enable */

#define MSR_DCD		(1 << 7)	/* Data Carrier Detect */
#define MSR_RI		(1 << 6)	/* Ring Indicator */
#define MSR_DSR		(1 << 5)	/* Data Set Ready */
#define MSR_CTS		(1 << 4)	/* Clear To Send */
#define MSR_DDCD	(1 << 3)	/* Delta Data Carrier Detect */
#define MSR_TERI	(1 << 2)	/* Trailing Edge Ring Indicator */
#define MSR_DDSR	(1 << 1)	/* Delta Data Set Ready */
#define MSR_DCTS	(1 << 0)	/* Delta Clear To Send */


#define LSR_FIFOE	(1 << 7)	/* FIFO Error Status */
#define LSR_TEMT	(1 << 6)	/* Transmitter Empty */
#define LSR_TDRQ	(1 << 5)	/* Transmit Data Request */
#define LSR_BI		(1 << 4)	/* Break Interrupt */
#define LSR_FE		(1 << 3)	/* Framing Error */
#define LSR_PE		(1 << 2)	/* Parity Error */
#define LSR_OE		(1 << 1)	/* Overrun Error */
#define LSR_DR		(1 << 0)	/* Data Ready */
                                                                  

#if defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) 
#define CR_UART_RBR	(0x00+RALINK_UART_LITE_BASE)
#define CR_UART_THR	(0x00+RALINK_UART_LITE_BASE)
#define CR_UART_IER	(0x04+RALINK_UART_LITE_BASE)
#define CR_UART_IIR	(0x08+RALINK_UART_LITE_BASE)
#define CR_UART_FCR	(0x08+RALINK_UART_LITE_BASE)
#define CR_UART_LCR	(0x0c+RALINK_UART_LITE_BASE)
#define CR_UART_MCR	(0x10+RALINK_UART_LITE_BASE)
#define CR_UART_LSR	(0x14+RALINK_UART_LITE_BASE)
#define CR_UART_MSR	(0x18+RALINK_UART_LITE_BASE)
#define CR_UART_SCR	(0x1c+RALINK_UART_LITE_BASE)
#define CR_UART_BRDL	(0x00+RALINK_UART_LITE_BASE)
#define CR_UART_BRDH	(0x04+RALINK_UART_LITE_BASE)
#define CR_UART_WORDA	(0x20+RALINK_UART_LITE_BASE)
#define CR_UART_HWORDA	(0x28+RALINK_UART_LITE_BASE)
#define CR_UART_MISCC	(0x24+RALINK_UART_LITE_BASE)
#define CR_UART_XYD	(0x2c+RALINK_UART_LITE_BASE)
#endif
