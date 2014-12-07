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
*  File Name: lcd.h
*     Author: Lee-John Fernandes
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    11/09/00  LRF   Created to move LCD functions from surf.h
*    11/16/00  MAS   Took out Null Functions (due to new sim bit)
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains all the definitions
//    of the Liquid Crystal Display (LCD) controller block. 
//
// Sp. Notes:
//
//    LCD_BUSY refers to the LCD block (Palmchip IP) busy state while 
//    LCD_DEV_BUSY refers to the LCD device (Physical display) busy state.
//
//    The simparam register is a pseudo register and exists only in 
//    simulation to allow verification of the timing parameters.  In 
//    simulation, the simparam register must be loaded with the same value as 
//    the param register to prevent false errors from being reported.  The
//    simparam register must not be accessed in real hardware.
//
/******************************************************************************/

#ifndef LCD_H
#define LCD_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "product.h"
#include "pubdefs.h"
#include "timer.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* LCD Block Specific Defines */
/* LCD Status Register bit definitions */
#define LCD_BUSY		  (0x0001)	/* Block busy */
#define LCD_FINISH                (0x0002)      /* Transfer complete */
#define LCD_HOLD_EMPTY            (0x0004)      /* Hold register empty */
#define LCD_DISP_ERROR            (0x0008)      /* Data error */
#define LCD_DISP_URUN             (0x0010)      /* Data underrun */
#define LCD_DISP_ORUN             (0x0020)      /* Data overrun */
#define LCD_DISP_MAX_DEPTH        (2)           /* Maximum data depth */
#define LCD_DISP_DEPTH_MASK       (0x0003)      /* Data depth  mask */
#define LCD_DISP_DEPTH_SHIFT      (6)           /* Data depth shift */
#define LCD_CTRL_URUN             (0x0100)      /* Control underun */
#define LCD_CTRL_ORUN             (0x0200)      /* Control overrun */
#define LCD_CTRL_MAX_DEPTH        (2)           /* Maximum control depth */
#define LCD_CTRL_DEPTH_MASK       (0x0003)      /* Control depth mask */
#define LCD_CTRL_DEPTH_SHIFT      (10)          /* Control depth shift */
#define LCD_ALL_STATUS_MASK       (0x033E)      /* Status bits mask */


/* LCD Interrupt Register/LCD Interrupt Enable Register bit definitions */
#define LCD_FINISH_INT            (0x0002)      /* Transfer complete int */
#define LCD_HOLD_EMPTY_INT        (0x0004)      /* Hold register empty int */
#define LCD_DISP_ERROR_INT        (0x0008)      /* Data error int */
#define LCD_DISP_URUN_INT         (0x0010)      /* Data underrun int */
#define LCD_DISP_ORUN_INT         (0x0020)      /* Data overrun int */
#define LCD_CTRL_URUN_INT         (0x0100)      /* Control underun int */
#define LCD_CTRL_ORUN_INT         (0x0200)      /* Control overrun int */
#define LCD_ALL_INT_MASK          (0x033E)      /* Mask all interrupts */


/* LCD Config Register bit defintions */
#define LCD_ENABLE_POL            (0x0001)     /* Enable pin polarity */
#define LCD_REG_STROBE_POL        (0x0002)     /* Reg strobe pin polarity */
#define LCD_RWDIR_POL             (0x0004)     /* Read/Write direction  pin 
					          polarity */
#define LCD_CHIP_SEL_POL          (0x0008)     /* Chip select pin polarity */
#define LCD_DISP_LENGTH           (0x0010)     /* Display data bit length */
#define LCD_CTRL_LENGTH           (0x0020)     /* Control data bit length */ 
#define LCD_BACKLIGHT_MODE        (0x4000)     /* Anode Push Pull or Open 
                                                  Collector mode */
#define LCD_BACKLIGHT_ENABLE      (0x8000)     /* Anode backlight enable */


/* LCD Parameter Register bit definitions */
#define LCD_SETUPVAL_MASK        (0x000F)       /* Setup value mask */
#define LCD_SETUPVAL_SHIFT	 (0)            /* Setup value shift */
#define LCD_HOLDVAL_MASK         (0x000F)       /* Hold value mask */
#define LCD_HOLDVAL_SHIFT	 (4)            /* Hold value shift */
#define LCD_EPWVAL_MASK          (0x00FF)       /* Enable pulse width value 
                                                   mask */
#define LCD_EPWVAL_SHIFT	 (8)            /* Enable pulse width value 
                                                   shift */
#define LCD_RECOVVAL_MASK        (0xFFFF)       /* Recovery value mask */
#define LCD_RECOVVAL_SHIFT	 (16)           /* Recovery value shift */


/* LCD Control Data Register bit definitions */ 
/* Note: Reading the Control Data Register returns the status of the 
**      actual LCD device. 
*/
#define LCD_REQREAD             (0x10000)      /* Request read - used in both 
                                                  Control and Display Data 
                                                  Registers */   
#define LCD_CTRL_MASK           (0x00FF)       /* Control data mask */
#define LCD_CTRL_SHIFT          (0)            /* Control data shift */
#define LCD_DEV_BUSY            (0x0080)       /* Device busy */


/* LCD Display Data Register bit definitions */
#define LCD_DISP_MASK           (0x00FF)       /* Display data mask */
#define LCD_DISP_SHIFT          (0)            /* Display data shift */




/* Hantronix specific defintions */
/* Initial Paramater values */
#define LCD_INIT_SETUPVAL	((uint32) 0x0002)  /* 2 Clocks */
#define LCD_INIT_EPWVAL		((uint32) 0x0008)  /* 8 Clocks */
#define LCD_INIT_HOLDVAL	((uint32) 000001)  /* 1 Clocks */
#define LCD_INIT_RECOVVAL	((uint32) 0x0800)  /* 2048 Clocks */

#define LCD_INIT_PARAM		( LCD_INIT_SETUPVAL << LCD_SETUPVAL_SHIFT  \
				| LCD_INIT_EPWVAL << LCD_EPWVAL_SHIFT	   \
				| LCD_INIT_HOLDVAL << LCD_HOLDVAL_SHIFT	   \
				| LCD_INIT_RECOVVAL << LCD_RECOVVAL_SHIFT )


/* Initial Config values */
#define LCD_INIT_CONFIG         ( LCD_DISP_LENGTH | LCD_CTRL_LENGTH )


/* Initial delay values */
#define LCD_INIT_DELAY1_US 400000   /* LCD delay 1 (micro secs) */
#define LCD_INIT_DELAY2_US 100000   /* LCD delay 1 (micro secs) */
#define LCD_INIT_DELAY3_US 2000   /* LCD delay 1 (micro secs) */


/* Device functions/dimensions */
#define	LCD_NUMLINES		(4)	/* Columns of characters */
#define	LCD_NUMCHARS		(16)	/* Rows of characters */
#define LCD_FUNSET		(0x38)	/* 8 Bit, 4 Line, 5x7 Pixel */
#define LCD_DISPON		(0x0C)	/* Disp On, Cursor & Blink Off */
#define LCD_DISPCLR		(0x01)	/* Clear Display, Home Cursor */
#define LCD_EMODESET		(0x06)	/* Cursor Auto-Incremental */
#define LCD_ADDRDRAM            (0x80)  /* Change cursor position */
#define LCD_AC_MASK             (0x7F)	/* Bit mask of Addrss Counter */



/*=====================*
 *  Type Defines       *
 *=====================*/
/* LCD Block Register Structure */
typedef struct lcdRegs_t 
{
	volatile uint32 lcdstat;	/* 0x00 */
	volatile uint32 lcdint;		/* 0x04 */
	volatile uint32 lcdinten;	/* 0x08 */
	volatile uint32 reserved1[5] ;	/* 0x0C */
	volatile uint32 lcdcfg;		/* 0x20 */
	volatile uint32 lcdparam;	/* 0x24 */
	volatile uint32 reserved2[2] ;	/* 0x28 */
	volatile uint32 lcdctrl;	/* 0x30 */
	volatile uint32 lcddata;	/* 0x34 */
        volatile uint32 reserved3[42];  /* 0x38 */
        volatile uint32 lcdsimparam;    /* 0xE0 - for testing in simulation */
} lcdRegs;


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void LcdInit ( lcdRegs *lcdPtr, tmrBlkRegs *tmrPtr, int timerNum);
PUBLIC void LcdClrscr ( lcdRegs *lcdPtr );
PUBLIC void LcdSetpos ( lcdRegs *lcdPtr, uint32 line, uint32 charpos );
PUBLIC uint32 LcdGetpos ( lcdRegs *lcdPtr );
PUBLIC void LcdPutchar ( lcdRegs *lcdPtr, int8 thechar );
PUBLIC void LcdPuthex ( lcdRegs *lcdPtr, int8 hexchar );
PUBLIC void LcdDispDec ( lcdRegs *lcdPtr, uint32 hexValue, uint32 decPlaces );
PUBLIC void LcdPrintString ( lcdRegs *lcdPtr, const int8 *thestring );
PUBLIC void LcdClearLine ( lcdRegs *lcdPtr, uint32 line );


/*=====================*
 *  Macro Functions    *
 *=====================*/

/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetStatus()
//
// SYNOPSIS       uint32 LcdGetStatus ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         uint32: Status
//
// DESCRIPTION    Returns contents of status register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetStatus(lcdPtr)                                                \
    ((lcdPtr)->lcdstat)	



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdClearStatus()
//
// SYNOPSIS       void LcdClearStatus ( lcdRegs *lcdPtr, uint32 bit )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 bit: Bits to be cleared 
//
// OUTPUT         none
//
// DESCRIPTION    Clears status register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdClearStatus(lcdPtr,bit)                                          \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdint |= (bit);					            \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsBusy()
//
// SYNOPSIS       bool LcdIsBusy ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Busy status
//
// DESCRIPTION    Returns TRUE if LCD Block is busy
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsBusy(lcdPtr)                                                   \
    (((lcdPtr)->lcdstat & LCD_BUSY) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsXferComplete()
//
// SYNOPSIS       bool LcdIsXferComplete ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Complete status
//
// DESCRIPTION    Returns TRUE if LCD transfer is complete
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsXferComplete(lcdPtr)                                           \
    (((lcdPtr)->lcdstat & LCD_FINISH) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsHoldRegEmpty()
//
// SYNOPSIS       bool LcdIsHoldRegEmpty ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Control/Display hold register empty status
//
// DESCRIPTION    Returns TRUE if Control/Display hold register is empty
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsHoldRegEmpty(lcdPtr)                                           \
    (((lcdPtr)->lcdstat & LCD_HOLD_EMPTY) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsDispXferError()
//
// SYNOPSIS       bool LcdIsDispXferError ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Data error status
//
// DESCRIPTION    Returns TRUE if LCD transfer data error occurs
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsDispXferError(lcdPtr)                                          \
    (((lcdPtr)->lcdstat & LCD_DISP_ERROR) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsDispURunError()
//
// SYNOPSIS       bool LcdIsDispURunError ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Display data underrun error status
//
// DESCRIPTION    Returns TRUE if LCD display data underrun occurs
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsDispURunError(lcdPtr)                                          \
    (((lcdPtr)->lcdstat & LCD_DISP_URUN) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsDispORunError()
//
// SYNOPSIS       bool LcdIsDispORunError ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Display data overrun error status
//
// DESCRIPTION    Returns TRUE if LCD display data overrun occurs
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsDispORunError(lcdPtr)                                          \
    (((lcdPtr)->lcdstat & LCD_DISP_ORUN) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetDispDepth()
//
// SYNOPSIS       uint32 LcdGetDispDepth ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         uint32: Display data register depth
//
// DESCRIPTION    Returns the depth of the display data register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetDispDepth(lcdPtr)                                             \
    (((lcdPtr)->lcdstat >> LCD_DISP_DEPTH_SHIFT) & LCD_DISP_DEPTH_MASK)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsCtrlURunError()
//
// SYNOPSIS       bool LcdIsCtrlURunError ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Control data underrun error status
//
// DESCRIPTION    Returns TRUE if LCD control data underrun occurs
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsCtrlURunError(lcdPtr)                                          \
    (((lcdPtr)->lcdstat & LCD_CTRL_URUN) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdIsCtrlORunError()
//
// SYNOPSIS       bool LcdIsCtrlORunError ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         bool: Control data overrun error status
//
// DESCRIPTION    Returns TRUE if LCD control data overrun occurs
//
// NOTE           None.
//
/******************************************************************************/
#define LcdIsCtrlORunError(lcdPtr)                                          \
    (((lcdPtr)->lcdstat & LCD_CTRL_ORUN) ? TRUE: FALSE)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetCtrlDepth()
//
// SYNOPSIS       uint32 LcdGetCtrlDepth ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         uint32: Control data register depth
//
// DESCRIPTION    Returns the depth of the control data register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetCtrlDepth(lcdPtr)                                             \
    (((lcdPtr)->lcdstat >> LCD_CTRL_DEPTH_SHIFT) & LCD_CTRL_DEPTH_MASK)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetIntStatus()
//
// SYNOPSIS       uint32 LcdIntStatus ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         uint32: Interrupt status
//
// DESCRIPTION    Returns contents of LCD interrupt status
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetIntStatus(lcdPtr)                                             \
    ((lcdPtr)->lcdint)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdClearIntStatus()
//
// SYNOPSIS       void LcdClearIntStatus ( lcdRegs *lcdPtr, uint32 int )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 int: Interrupt to be cleared
//
// OUTPUT         none
//
// DESCRIPTION    Clears status of specified interrupts
//
// NOTE           None.
//
/******************************************************************************/
#define LcdClearIntStatus(lcdPtr,int)                                       \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdint |= (uint32)(int);					    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetIntEnable()
//
// SYNOPSIS       uint32 LcdIntEnable ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//
// OUTPUT         uint32: Interrupts that are enabled
//
// DESCRIPTION    Returns contents of LCD interrupt enable register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetIntEnable(lcdPtr)                                             \
    ((lcdPtr)->lcdinten)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdDisableInt()
//
// SYNOPSIS       void LcdDisableInt ( lcdRegs *lcdPtr, uint32 int )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 int: Interrupt to be disabled
//
// OUTPUT         none
//
// DESCRIPTION    Disables specified interrupts
//
// NOTE           None.
//
/******************************************************************************/
#define LcdDisableInt(lcdPtr,int)                                           \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
   (lcdPtr)->lcdinten &= (uint32)(~(int));				    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdEnableInt()
//
// SYNOPSIS       void LcdEnableInt ( lcdRegs *lcdPtr, uint32 int )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 int: Interrupt to be enabled
//
// OUTPUT         none
//
// DESCRIPTION    Enables specified interrupts
//
// NOTE           None.
//
/******************************************************************************/
#define LcdEnableInt(lcdPtr,int)                                            \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdinten |= (uint32)(int);			       	    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetConfig()
//
// SYNOPSIS       uint32 LcdGetConfig ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32 config: Config
//
// DESCRIPTION    Returns contents of LCD config register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetConfig(lcdPtr)                                                \
    ((lcdPtr)->lcdcfg)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdClearConfig()
//
// SYNOPSIS       void LcdClearConfig ( lcdRegs *lcdPtr, uint32 ConfigBits )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 configBits: Config bits to be clear 
//                
// OUTPUT         none
//
// DESCRIPTION    Clears specified config bits
//
// NOTE           None.
//
/******************************************************************************/
#define LcdClearConfig(lcdPtr,configBits)                                   \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdcfg &= (uint32) (~(configBits));		       	    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdSetConfig()
//
// SYNOPSIS       void LcdSetConfig ( lcdRegs *lcdPtr, uint32 ConfigBits )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 configBits: Config bits to be set 
//                
// OUTPUT         none
//
// DESCRIPTION    Sets specified config bits
//
// NOTE           None.
//
/******************************************************************************/
#define LcdSetConfig(lcdPtr,configBits)                                     \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdcfg |= (uint32)(configBits);				    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadConfig()
//
// SYNOPSIS       void LcdLoadConfig ( lcdRegs *lcdPtr, uint32 ConfigBits )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 configBits: Config bits to be set 
//                
// OUTPUT         none
//
// DESCRIPTION    Loads value into Config register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadConfig(lcdPtr,configBits)                                    \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdcfg = (uint32)(configBits);				    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetAllParamVals()
//
// SYNOPSIS       void LcdGetAllParamVals ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32: Parameter values
//
// DESCRIPTION    Returns contents of parameter register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetAllParamVals(lcdPtr)                                          \
    ((lcdPtr)->lcdparam)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadAllParamVals()
//
// SYNOPSIS       void LcdLoadAllParamVals ( lcdRegs *lcdPtr, uint32 paramVal )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 paramVal: Parameter value to be set
//                
// OUTPUT         none
//
// DESCRIPTION    Returns contents of parameter register
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadAllParamVals(lcdPtr,paramVal)                                \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdparam = (paramVal);					    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadSetupParamVal()
//
// SYNOPSIS       void LcdLoadSetupParamVal ( lcdRegs *lcdPtr, uint32 paramVal )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 paramVal: Parameter value to be set
//                
// OUTPUT         none
//
// DESCRIPTION    Loads setup parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadSetupParamVal(lcdPtr,paramVal)                               \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdparam = ( ((lcdPtr)->lcdparam &                            \
			  (~(LCD_SETUPVAL_MASK << LCD_SETUPVAL_SHIFT))) |   \
			 (((paramVal) & LCD_SETUPVAL_MASK) <<               \
                          LCD_SETUPVAL_SHIFT) );                            \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetSetupParamVal()
//
// SYNOPSIS       uint32 LcdGetSetupParamVal ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32: Setup prameter value
//
// DESCRIPTION    Returns setup parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetSetupParamVal(lcdPtr)                                         \
    (((lcdPtr)->lcdparam >> LCD_SETUPVAL_SHIFT) & LCD_SETUPVAL_MASK)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadHoldParamVal()
//
// SYNOPSIS       void LcdLoadHoldParamVal ( lcdRegs *lcdPtr, uint32 paramVal )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 paramVal: Parameter value to be set
//                
// OUTPUT         none
//
// DESCRIPTION    Loads Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadHoldParamVal(lcdPtr,paramVal)                                \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdparam = ( ((lcdPtr)->lcdparam &                            \
			  (~(LCD_HOLDVAL_MASK << LCD_HOLDVAL_SHIFT))) |     \
			 (((paramVal) & LCD_HOLDVAL_MASK) <<                \
			  LCD_HOLDVAL_SHIFT) );				    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetHoldParamVal()
//
// SYNOPSIS       uint32 LcdGetHoldParamVal ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32: Enable Pulse Width prameter value
//
// DESCRIPTION    Returns Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetHoldParamVal(lcdPtr)                                          \
    (((lcdPtr)->lcdparam >> LCD_HOLDVAL_SHIFT) & LCD_HOLDVAL_MASK)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadEpwParamVal()
//
// SYNOPSIS       void LcdLoadEpwParamVal ( lcdRegs *lcdPtr, uint32 paramVal )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 paramVal: Parameter value to be set
//                
// OUTPUT         none
//
// DESCRIPTION    Loads Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadEpwParamVal(lcdPtr,paramVal)                                 \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdparam = ( ((lcdPtr)->lcdparam &                            \
			  (~(LCD_EPWVAL_MASK << LCD_EPWVAL_SHIFT))) |       \
			 (((paramVal) & LCD_EPWVAL_MASK) <<                 \
			  LCD_EPWVAL_SHIFT) );			            \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetEpwParamVal()
//
// SYNOPSIS       uint32 LcdGetEpwParamVal ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32: Enable Pulse Width prameter value
//
// DESCRIPTION    Returns Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetEpwParamVal(lcdPtr)                                           \
    (((lcdPtr)->lcdparam >> LCD_EPWVAL_SHIFT) & LCD_EPWVAL_MASK)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdLoadRecovParamVal()
//
// SYNOPSIS       void LcdLoadRecovParamVal ( lcdRegs *lcdPtr, 
//                                            uint32 paramVal )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                uint32 paramVal: Parameter value to be set
//                
// OUTPUT         none
//
// DESCRIPTION    Loads Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdLoadRecovParamVal(lcdPtr,paramVal)                               \
  /* do-while added for macro safety */                                     \
  do									    \
  { 									    \
    (lcdPtr)->lcdparam = ( ((lcdPtr)->lcdparam &                            \
			  (~(((uint32)LCD_RECOVVAL_MASK) <<                 \
                               LCD_RECOVVAL_SHIFT))) |                      \
			  (((uint32)((paramVal) & LCD_RECOVVAL_MASK)) <<    \
			    LCD_RECOVVAL_SHIFT) );	       	            \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LcdGetRecovParamVal()
//
// SYNOPSIS       uint32 LcdGetRecovParamVal ( lcdRegs *lcdPtr )
//
// TYPE           Macro 
//
// INPUT          lcdRegs *lcdPtr: Base Pointer of the LCD device
//                
// OUTPUT         uint32: Enable Pulse Width prameter value
//
// DESCRIPTION    Returns Enable Pulse Width parameter value
//
// NOTE           None.
//
/******************************************************************************/
#define LcdGetRecovParamVal(lcdPtr)                                         \
    (((lcdPtr)->lcdparam >> LCD_RECOVVAL_SHIFT) & LCD_RECOVVAL_MASK)


#endif /* LCD_H */

