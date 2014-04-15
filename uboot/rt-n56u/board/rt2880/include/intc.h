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
*  File Name: intc.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/15/97  RWB   Started.
*    10/23/00  IST   Merged header files for PalmPak 2.0
*    10/31/00  IST   Updated Interrupt bit layout for PPv2 hardware
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    The file contains all the interrupt controller block definitions.
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef INTC_H
#define INTC_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"

#include "aux_intc.h"


/*=====================*
 *  Defines            *
 *=====================*/
 
/*
** $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
** NOTE:
** If you changed the bit positions on the interrupt, then please change the
** intCode enum below according to the bit positions, other wise the exception 
** handling will not work properly.
** The enums are defined from LSB to MSB order.
** $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
*/
/* bit positions of the interrupts */
#define INTC_TIMER0_INT                 (0x00000001)
#define INTC_WDTIMER_INT                (0x00000002)
#define INTC_UART0_INT                  (0x00000004)
#define INTC_PIO_INT                    (0x00000008)
#define INTC_BTN_INT			(0x00000010)
#define INTC_LCD_INT                    (0x00000020)
#define INTC_DMA_AUX0_INT               (0x00000040)
#define INTC_DMA_AUX1_INT               (0x00000080)
#define INTC_UARTLITE_INT               (0x00000100)
#define INTC_RTC_TMR_INT                (0x00000200)
#define INTC_RTC_ALARM_INT              (0x00000400)
#define INTC_EXT0_INT                   (0x00000800)
#define INTC_EXT1_INT                   (0x00001000)
#define INTC_AUX0_INT                   (0x00002000)
#define INTC_AUX1_INT                   (0x00004000)
#define INTC_AUX2_INT                   (0x00008000)
#define INTC_AUX3_INT                   (0x00010000)
#define INTC_AUX4_INT                   (0x00020000)
#define INTC_AUX5_INT                   (0x00040000)
#define INTC_AUX6_INT                   (0x00080000)
#define INTC_AUX7_INT                   (0x00100000)
#define INTC_AUX8_INT                   (0x00200000)
#define INTC_AUX9_INT                   (0x00400000)
#define INTC_AUX10_INT                  (0x00800000)

#define GLOBAL_INT                      (0x80000000)
#define ALL_INTS                        (0x00FFFFFF)    /* Excluding global interrupt bit */


/*=====================*
 *  Type defines       *
 *=====================*/

typedef struct intcRegs_t 
{
        volatile uint32 irq0Status;    /* 0x00 */
        volatile uint32 irq1Status;
        volatile uint32 reserved[6];
        volatile uint32 intType;       /* 0x20 */
        volatile uint32 reserved1[3];
        volatile uint32 rawStatus;     /* 0x30 */
        volatile uint32 intEnable;
        volatile uint32 intDisable;
} intcRegs;


typedef enum
{
  timerInt,
  watchdogInt,
  uartInt,
  pioInt,
  btnInt,
  lcdInt,
  dmaAux0Int,
  dmaAux1Int,
  uartliteInt,
  rtcTmrInt,
  rtcAlarmInt,
#if 1	// currently unused interrupts -- add in as necessary.
  ext0Int,
  ext1Int,
  aux0Int,
  aux1Int,
  aux2Int,
  aux3Int,
  aux4Int,
  aux5Int,
  aux6Int,
  aux7Int,
  aux8Int,
  aux9Int,
  aux10Int,
#endif
  maxInts
} intCode;

/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/

/*=====================*
 *  Macro Functions    *
 *=====================*/

/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcEnableInt()
//
// SYNOPSIS       void IntcEnableInt ( uint32 val )
//
// TYPE           Inline function
//
// INPUT          uint32 val: Contains one or more bits corresponding to the 
//                            requested interrupts  
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the requested interrupt.
//
// NOTE           Use the interrupt bits defined in the intc.h file.  
//
/******************************************************************************/
#define IntcEnableInt(val)					      	\
  /* do-while added for macro safety */					\
  do									\
  { 									\
    ((intcRegs *)(INTC_BASE))->intEnable = (val); 			\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcIsIntEnable()
//
// SYNOPSIS       uint32 IntcIsIntEnable ( uint32 val )
//
// TYPE           Inline function
//
// INPUT          uint32 val: Contains one or more bits corresponding to the 
//                            requested interrupts  
//
// OUTPUT         None
//
// DESCRIPTION    This function returns a non-zero value if the given interrupt
//                is enabled. Otherwise returns zero. The purpose of this 
//                function is to check single interrupt at a time. But if the
//                caller wants to check the presence of one of the multiple
//                interrupts, it can be done easily by checking return value.
//                If the caller wants to check whether all the requested 
//                interrupts have occurred or not, then the return value must be
//                checked by the caller.
//
// NOTE           Use the interrupt bits defined in the intc.h file.
//
/******************************************************************************/
#define IntcIsIntEnable(val)	( ((intcRegs *)INTC_BASE)->intEnable & (val) )


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcDisableInt()
//
// SYNOPSIS       void IntcDisableInt ( uint32 val )
//
// TYPE           Inline function
//
// INPUT          uint32 val: Contains one or more bits corresponding to the 
//                            requested interrupts  
//
// OUTPUT         None
//
// DESCRIPTION    This function disables the requested interrupt.
//
// NOTE           Use the interrupt bits defined in the intc.h file.  
//
/******************************************************************************/
#define IntcDisableInt(val)							\
  /* do-while added for macro safety */					\
  do									\
  { 									\
    ((intcRegs *)(INTC_BASE))->intDisable = (val); 			\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcEnableAllInt()
//
// SYNOPSIS       void IntcEnableAllInt ( void )
//
// TYPE           Inline function
//
// INPUT          None  
//
// OUTPUT         None
//
// DESCRIPTION    This function enables all the unmasked interrupts.
//
// NOTE           Use the interrupt bits defined in the intc.h file. 
//                This function preserves the previously masked/disabled 
//                interrupt bits. 
//
/******************************************************************************/
#define IntcEnableAllInt()					       	\
  /* do-while added for macro safety */					\
  do									\
  { 									\
    ((intcRegs *)(INTC_BASE))->intEnable = GLOBAL_INT; 			\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcDisableAllInt()
//
// SYNOPSIS       void IntcDisableAllInt ( void )
//
// TYPE           Inline function
//
// INPUT          None  
//
// OUTPUT         None
//
// DESCRIPTION    This function enables all the unmasked interrupts.
//
// NOTE           Use the interrupt bits defined in the intc.h file. 
//                This function preserves the previously masked/disabled 
//                interrupt bits. 
//
/******************************************************************************/
#define IntcDisableAllInt()					       	\
  /* do-while added for macro safety */					\
  do									\
  { 									\
    ((intcRegs *)(INTC_BASE))->intDisable = GLOBAL_INT; 		\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcSetIrqInt()
//
// SYNOPSIS       void IntcSetIrqInt ( uint32 Val, uint32 IrqNum )
//
// TYPE           Inline function
//
// INPUT          uint32 Val:    Contains one or more bits corresponding to
//                               the requested interrupts  
// INPUT          uint32 IrqNum: IRQ signal index
//
// OUTPUT         None
//
// DESCRIPTION    Steers the requested interrupt to requested IRQ signal.
//
// NOTE           Use the interrupt bits defined in the intc.h file.  
//
/******************************************************************************/
#define IntcSetIrqInt(val,irqNum)			       		\
  /* do-while added for macro safety */					\
  do {									\
    /* REVISIT: HW and SW handles exactly 2 Irq signals currently. */	\
    if ((irqNum) == 0)							\
    {									\
        ((intcRegs *)(INTC_BASE))->intType &= ~(val);			\
    }									\
    else								\
    {									\
        ((intcRegs *)(INTC_BASE))->intType |= (val);			\
    }									\
  } while (0)


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcCheckIrq0Int()
//
// SYNOPSIS       bool IntcCheckIrq0Int ( uint32 val )
//
// TYPE           Inline function
//
// INPUT          uint32 Val:  Contains one or more bits corresponding to the 
//                             requested interrupts  
//
// OUTPUT         Nonzero if one or more of the requested interrupts has
//                occurred.
//
// DESCRIPTION    This function checks whether the requested IRQ interrupt has
//                occurred or not. If present, then returns TRUE otherwise 
//                FALSE.
//
// NOTE           Use the interrupt bits defined in the intc.h file.
//
/******************************************************************************/
#define IntcCheckIrq0Int(val) ((bool)(((intcRegs*)(INTC_BASE))->irq0Status & (val)))


/* FUNCTION_DESC **************************************************************/
//
// NAME           IntcCheckIrq1Int()
//
// SYNOPSIS       bool IntcCheckIrq1Int ( uint32 val )
//
// TYPE           Inline function
//
// INPUT          uint32 Val:  Contains one or more bits corresponding to the 
//                             requested interrupts  
//
// OUTPUT         Nonzero if one or more of the requested interrupts has
//                occurred.
//
// DESCRIPTION    This function checks whether the requested IRQ interrupt has
//                occurred or not. If present, then returns TRUE otherwise 
//                FALSE.
//
// NOTE           Use the interrupt bits defined in the intc.h file.
//
/******************************************************************************/
#define IntcCheckIrq1Int(val) ((bool)(((intcRegs*)(INTC_BASE))->irq1Status & (val)))


#endif /* INTC_H */
