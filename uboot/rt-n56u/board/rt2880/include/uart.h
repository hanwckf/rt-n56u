/******************************************************************************
 *  This program is free software; you can redistribute  it and/or modify it                                                                                           *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your                                                                                           *  option) any later version.
 *                                                                                                                                                                     *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF                                                                                           *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,                                                                                            *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF                                                                                           *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT                                                                                           *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                                                                  *
 *  You should have received a copy of the  GNU General Public License along                                                                                           *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.                                                                                                                            *
 */

/*******************************************************************************
*
*  File Name: uart.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/18/97  RWB   ...
*    10/19/00  IST   Merged header files for PalmPak 2.0
*    05/20/02  LYT   Changed formatting to conform to standard format.
*
*
*******************************************************************************/

/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the UART block definitions. 
//
// Sp. Notes:  
//
/******************************************************************************/

#ifndef UART_H
#define UART_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/

/* Interrupt Enable register bit definitions */
#define UART_DATA_AVAIL_ID_INT		(0x01)
#define UART_TX_BUF_EMPTY_ID_INT	(0x02)
#define UART_LINE_ID_INT		(0x04)
#define UART_MODEM_ID_INT		(0x08)

#define UART_INT_MASK			(0x000F)

/* Interrupt ID register bit definitions */
#define UART_INT_PENDING	(0x01)
#define UART_ID_MASK		(0x0F)
#define UART_FIFOS_ENABLED_MASK	(0xC0)

#define UART_NO_ID		(0x01)	/* No interrupts */
#define UART_LINE_STAT_ID	(0x06)	/* Highest priority */
#define UART_RX_DATA_AVAIL_ID	(0x04)	/* Second priority */
#define UART_CHAR_TIMEOUT_ID	(0x0C)	/* Second priority */
#define UART_TX_BUF_EMPTY_ID	(0x02)	/* Third priority */
#define UART_MODEM_STAT_ID	(0x00)	/* Fourth priority */

#define UART_FIFOS_NONE		(0x00)	/* Fifos not enabled or not present */
#define UART_FIFOS_UNUSABLE	(0x80)	/* 16550 UART only */
#define UART_FIFOS_ENABLED	(0xC0)

/* FIFO Control register bit definitions */
#define UART_FIFO_ENABLE	(0x01)
#define UART_RX_FIFO_RESET	(0x02)
#define UART_TX_FIFO_RESET	(0x04)
#define UART_DMA_ENABLE		(0x08)

#define UART_TX_BUF_TRIG_SHIFT	(4)
#define UART_TX_BUF_TRIG_MASK	(0x3 << UART_TX_BUF_TRIG_SHIFT)
#define UART_TX_TRIG_AT_0	(0 << UART_TX_BUF_TRIG_SHIFT)
#define UART_TX_TRIG_AT_4	(1 << UART_TX_BUF_TRIG_SHIFT)
#define UART_TX_TRIG_AT_8	(2 << UART_TX_BUF_TRIG_SHIFT)
#define UART_TX_TRIG_AT_12	(3 << UART_TX_BUF_TRIG_SHIFT)

#define UART_RX_BUF_TRIG_SHIFT	(6)
#define UART_RX_BUF_TRIG_MASK	(0x3 << UART_RX_BUF_TRIG_SHIFT)
#define UART_RX_TRIG_AT_1	(0 << UART_RX_BUF_TRIG_SHIFT)
#define UART_RX_TRIG_AT_4	(1 << UART_RX_BUF_TRIG_SHIFT)
#define UART_RX_TRIG_AT_8	(2 << UART_RX_BUF_TRIG_SHIFT)
#define UART_RX_TRIG_AT_14	(3 << UART_RX_BUF_TRIG_SHIFT)

/* LINE Control register bit definitions */
#define UART_DATA_BIT_MASK	(0x03)
#define UART_DATA_BIT_5		(0x00)
#define UART_DATA_BIT_6		(0x01)
#define UART_DATA_BIT_7		(0x02)
#define UART_DATA_BIT_8		(0x03)
#define UART_STOP_BITS_2	(0x04) /* 1.5 bits if char len = 5 */
#define UART_PARITY_ENABLE	(0x08)
#define UART_EVEN_PARITY	(0x10)
#define UART_FORCE_PARITY	(0x20)
#define UART_FORCE_BREAK	(0x40)

/* MODEM Control register bit definitions */
#define UART_DTR_SIGNAL		(0x01)
#define UART_RTS_SIGNAL		(0x02)
#define UART_OUT1_SIGNAL	(0x04)
#define UART_OUT2_SIGNAL	(0x08)
#define UART_LOOP_MODE		(0x10)

/* Line Status register bit definitions */
#define UART_DATA_READY		(0x01)
#define UART_OVERRUN_ERR	(0x02)
#define UART_PARITY_ERR		(0x04)
#define UART_FRAMING_ERR	(0x08)
#define UART_BREAK_INT		(0x10)
#define UART_TX_BUF_EMPTY	(0x20)
#define UART_TX_EMPTY		(0x40)
#define UART_RX_FIFO_ERR	(0x80)

#ifdef USE_UART_FIFO
#define UART_LINE_ERRORS_MASK	( UART_OVERRUN_ERR | UART_OVERRUN_ERR \
				| UART_PARITY_ERR  | UART_FRAMING_ERR \
				| UART_BREAK_INT   | UART_RX_FIFO_ERR )
#else
#define UART_LINE_ERRORS_MASK	( UART_OVERRUN_ERR | UART_OVERRUN_ERR \
				| UART_PARITY_ERR  | UART_FRAMING_ERR \
				| UART_BREAK_INT )
#endif

/* Modem Status register bit definitions */
#define UART_DELTA_CTS_INT	(0x01)
#define UART_DELTA_DSR_INT	(0x02)
#define UART_TE_RI_INT		(0x04)
#define UART_DELTA_DCD_INT	(0x08)
#define UART_CTS_SIGNAL		(0x10)
#define UART_DSR_SIGNAL		(0x20)
#define UART_RI_SIGNAL		(0x40)
#define UART_DCD_SIGNAL		(0x80)

#define UART_MODEM_INT_MASK	( UART_DELTA_CTS_INT | UART_DELTA_DSR_INT \
				| UART_TE_RI_INT     | UART_DELTA_DCD_INT )


/* Scratch register bit definitions */
#define UART_SCRATCH_REG_MASK	(0xFF)


/* Clock Divider register bit definitions */
/*
**	Value which the Divisor Latch gets programmed with.
**	SYS_CLK_KHZ gives more significant digits than SYS_CLK_MHZ.
**	There is a 16 bit clock pre-divider.
*/

#define UART_BAUD_2400		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 2400)))
#define UART_BAUD_4800		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 4800)))
#define UART_BAUD_9600		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 9600)))
#define UART_BAUD_19200		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 19200)))
#define UART_BAUD_38400		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 38400)))
#define UART_BAUD_57600		((uint32)((SYS_CLK_KHZ * 1000) / (16 * 57600)))
#define UART_BAUD_115200	((uint32)((SYS_CLK_KHZ * 1000) / (16 * 115200)))
#define UART_BAUD_230400	((uint32)((SYS_CLK_KHZ * 1000) / (16 * 230400)))
#define UART_BAUD_460800	((uint32)((SYS_CLK_KHZ * 1000) / (16 * 460800)))
#define UART_BAUD_921600	((uint32)((SYS_CLK_KHZ * 1000) / (16 * 921600)))


/* Miscellaneous definitions */
#define UART_PARITY_NONE       	(0)
#define UART_PARITY_EVEN       	(1)
#define UART_PARITY_ODD	       	(2)

#define UART_TX_FIFO_DEPTH	(16)
#define UART_RX_FIFO_DEPTH	(16)


/*
**----------------------------------------------------------------------------
** This value is used with the UartGetChar() function. If there is any line 
** error then this bit will be set in the return value.
**----------------------------------------------------------------------------
*/
#define UART_DATA_ERROR	       (0x8000)


/*=====================*
 *  Type defines       *
 *=====================*/

typedef struct uartRegs_t 
{
	volatile uint32 rxData;		/* 0x0  */
	volatile uint32 txData;		/* 0x4  */
	volatile uint32 intEnable;	/* 0x8  */
	volatile uint32 intId;		/* 0xC  */
	volatile uint32 fifoControl;	/* 0x10 */
	volatile uint32 lineControl;	/* 0x14 */
	volatile uint32 modemControl;	/* 0x18 */
	volatile uint32 lineStatus;	/* 0x1C */
	volatile uint32 modemStatus;	/* 0x20 */
	volatile uint32 scratch;	/* 0x24 */
	volatile uint32 clkDivider;	/* 0x28 */
} uartRegs;

typedef enum uartDataBits_t 
{
	dataBits5,
	dataBits6,
	dataBits7,
	dataBits8
} uartDataBits;

typedef enum uartStopBits_t 
{
	stopBits1,
	stopBits2
} uartStopBits;

typedef enum uartParity_t 
{
	NoParity,
	EvenParity,
	OddParity
} uartParity;

typedef enum uartBaud_t 
{
	B2400,
	B4800,
	B9600,
	B19200,
	B38400,
	B57600,
	B115200,
	B230400,
	B460800,
	B921600
} uartBaud;

typedef enum uartFlowControl_t
{
	Hardware,
	XonXoff
} uartFlowControl;


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/

PUBLIC void UartSetBaud ( uartRegs *uPtr, uartBaud baud );
PUBLIC void UartSetDataBits ( uartRegs *uPtr, uartDataBits dBits );
PUBLIC void UartSetStopBits ( uartRegs *uPtr, uartStopBits stopBits );
PUBLIC void UartSetParity ( uartRegs *uPtr, uartParity parity );
PUBLIC void UartStickParity ( uartRegs *uPtr, bool set );
PUBLIC void UartSetRxFifoTrigger ( uartRegs *uPtr, uint32 trigLevel );
PUBLIC void UartSetTxFifoTrigger ( uartRegs *uPtr, uint32 trigLevel );
PUBLIC void UartConfigure ( uartRegs *uPtr, uint32 config );
PUBLIC uint32 UartPutChar ( uartRegs *uPtr, uint8 data );
PUBLIC uint32 UartGetChar ( uartRegs *uPtr );
PUBLIC void UartLoopbackMode ( uartRegs *uPtr, bool mode );


/*=====================*
 *  Macro Functions    *
 *=====================*/

/* FUNCTION_DESC **************************************************************/
//
// NAME           UartEnableIdInt()
//
// SYNOPSIS       void UartEnableIdInt ( uartRegs *uPtr, uint32 uInts )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//                uint32 uInts: uart ID interrupts
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the requested UART interrupts.
//
// NOTE           Use the interrupt bit definitions defined in uart.h.  
//
/******************************************************************************/
#define UartEnableIdInt(uPtr,uInts)					\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->intEnable |= (uInts); 					\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartDisableIdInt()
//
// SYNOPSIS       void UartDisableIdInt ( uartRegs *uPtr, uint32 uInts )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//                uint32 uInts: uart ID interrupts
//
// OUTPUT         None
//
// DESCRIPTION    This function disables the requested UART interrupts.
//
// NOTE           Use the interrupt bit definitions defined in uart.h.  
//
/******************************************************************************/
#define UartDisableIdInt(uPtr,uInts)					\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->intEnable &= ~(uInts);					\
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           UartGetIntId()
//
// SYNOPSIS       uint32 UartGetIntId ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         uint32: UART interrupt id
//
// DESCRIPTION    This function returns the existing UART interrupt id.
//
// NOTE           It is possible that for 16550 the bit 6-7 can be at logic 1
//                if the fifo is enabled, that is why the UART_ID_MASK is used. 
//
/******************************************************************************/
#define UartGetIntId(uPtr)	( (uint32)((uPtr)->intId & UART_ID_MASK) )


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartIsFifoEnabled()
//
// SYNOPSIS       bool UartIsFifoEnabled ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         bool: TRUE/FALSE
//
// DESCRIPTION    This function returns TRUE if the fifo's are enabled.
//                Otherwise, returns FALSE.
//
// NOTE           None
//
/******************************************************************************/
#define UartIsFifoEnabled(uPtr)						\
    ( ((uPtr)->intId & UART_FIFOS_ENABLED_MASK) == UART_FIFOS_ENABLED )


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartGetLineStatus()
//
// SYNOPSIS       void UartGetLineStatus ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         uint32: interrupt occurred
//
// DESCRIPTION    This function returns the line status of the requested UART.
//
// NOTE           None  
//
/******************************************************************************/
#define UartGetLineStatus(uPtr)	( (uPtr)->lineStatus )


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartGetLineControl()
//
// SYNOPSIS       void UartGetLineControl ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         uint32: contents of line control register
//
// DESCRIPTION    This function returns the line control register
//
// NOTE           None  
//
/******************************************************************************/
#define UartGetLineControl(uPtr)	( (uPtr)->lineControl )


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartConfigure()
//
// SYNOPSIS       void UartConfigure ( uartRegs *uPtr, uint32 config )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//                uint32 config: Configuraton value of the UART line control
//                               register  
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the requested configuration for the 
//                specified UART, by setting the UART line control register.
//                This is the quickest way to set all line/modem configuration.
//                But it is caller's responsility to set the appropriate bits
//                in the config value. Please make sure that the LOOPBACK
//                mode is set/clear appropriately.
//
// NOTE           This function does not perform any check or save old value. 
//
/******************************************************************************/
#define UartConfigure(uPtr, config)					\
  do									\
  {									\
    uPtr->lineControl = config;						\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartGetModemStatus()
//
// SYNOPSIS       void UartGetModemStatus ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         uint32: interrupt occurred
//
// DESCRIPTION    This function returns the line status of the requested UART.
//
// NOTE           None  
//
/******************************************************************************/
#define UartGetModemStatus(uPtr)	( (uPtr)->modemStatus )


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartAssertModemSignal()
//
// SYNOPSIS       void UartAssertModemSignal ( uartRegs *uPtr, uint32 sig )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//                uint32 sig: One of the UART signal such as DTR, RTS, etc.
//
// OUTPUT         None
//
// DESCRIPTION    This function asserts the given signal on the requested UART
//                port.
//
// NOTE           None.  
//
/******************************************************************************/
#define UartAssertModemSignal(uPtr,sig)					\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->modemControl |= (sig);					\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartDeassertModemSignal()
//
// SYNOPSIS       void UartDeassertModemSignal ( uartRegs *uPtr, uint32 sig )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//                uint32 sig: One of the UART signal such as DTR, RTS, etc.
//
// OUTPUT         None
//
// DESCRIPTION    This function deasserts the given signal on the requested UART
//                port.
//
// NOTE           None.  
//
/******************************************************************************/
#define UartDeassertModemSignal(uPtr,sig)	      			\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->modemControl &= ~(sig); 					\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartEnableFifo()
//
// SYNOPSIS       void UartEnableFifo ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the UART fifo.
//
// NOTE           None
//
/******************************************************************************/
#define UartEnableFifo(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl |= UART_FIFO_ENABLE;				\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartDisableFifo()
//
// SYNOPSIS       void UartDisableFifo ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function disables the UART fifo.
//
// NOTE           None
//
/******************************************************************************/
#define UartDisableFifo(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl &= ~UART_FIFO_ENABLE;				\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartResetRxFifo()
//
// SYNOPSIS       void UartResetRxFifo ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function resets the UART receive fifo and resets the
//                receiver status.  The shift register is not cleared.
//
// NOTE           None
//
/******************************************************************************/
#define UartResetRxFifo(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl |= UART_RX_FIFO_RESET;				\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartResetTxFifo()
//
// SYNOPSIS       void UartResetTxFifo ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function resets the UART transmit fifo and resets the
//                transmitter status.  The shift register is not cleared.
//
// NOTE           None
//
/******************************************************************************/
#define UartResetTxFifo(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl |= UART_TX_FIFO_RESET;				\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartResetFifos()
//
// SYNOPSIS       void UartResetFifos ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function resets the UART transmit and receive fifos and
//                resets the transmitter and receiver status.  The shift
//                registers are not cleared.
//
// NOTE           None
//
/******************************************************************************/
#define UartResetFifos(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl |= (UART_TX_FIFO_RESET | UART_RX_FIFO_RESET);	\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartEnableDma()
//
// SYNOPSIS       void UartEnableDma ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the data transfer DMA mode.
//
// NOTE           None
//
/******************************************************************************/
#define UartEnableDma(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl |= UART_DMA_ENABLE;				\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           UartDisableDma()
//
// SYNOPSIS       void UartDisableDma ( uartRegs *uPtr )
//
// TYPE           Inline function
//
// INPUT          uartRegs *uPtr: base address of the UART
//
// OUTPUT         None
//
// DESCRIPTION    This function disables the data transfer DMA mode.
//
// NOTE           None
//
/******************************************************************************/
#define UartDisableDma(uPtr)						\
  /* do-while added for macro safety */                                 \
  do									\
  { 									\
    (uPtr)->fifoControl &= ~UART_DMA_ENABLE;				\
  } while (0)


#endif /* UART_H */
