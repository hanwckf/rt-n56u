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
*  File Name: tmr.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/12/97  RWB   Created.
*    10/23/00  IST   Merged header files for PalmPak 2.0
*
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains all the timer block definitions. 
//
// Sp. Notes:
//
 ******************************************************************************/

#ifndef TIMER_H
#define TIMER_H


/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/

/* Common to all timers - used only in sysboot.s! */
#define TMR_STATUS_REG          (TMR_BASE)

/* Timer Status register bit definitions */
#define TMR_INT		       	(0x0001)
#define WD_INT		       	(0x0002)
#define TMR_INTS                (0x0003)
#define TMR_RESET	       	(0x0010)
#define WD_RESET	       	(0x0020)

/* Timer Control register bit definitions */

#define TMR_SCALE_MASK		(0xf)
#define TMR_NUM_SCALES          (0x10)

#define TMR_SCALE_DN_0		(0x0000)
#define TMR_SCALE_DN_4		(0x0001)
#define TMR_SCALE_DN_8		(0x0002)
#define TMR_SCALE_DN_16		(0x0003)
#define TMR_SCALE_DN_32		(0x0004)
#define TMR_SCALE_DN_64		(0x0005)
#define TMR_SCALE_DN_128	(0x0006)
#define TMR_SCALE_DN_256	(0x0007)

#define TMR_MODE_SHIFT		(4)
#define TMR_MODE_MASK		(0x3 << TMR_MODE_SHIFT)

#define TMR_FREE_RUN_MODE	(0x00)
#define TMR_PERIODIC_MODE	(0x01 << TMR_MODE_SHIFT)
#define TMR_TIME_OUT_MODE	(0x02 << TMR_MODE_SHIFT)        /* Not supported in AMBA */
#define TMR_WATCHDOG_MODE	(0x03 << TMR_MODE_SHIFT)	/* Not supported in AMBA */

#define TMR_ENABLE	       	(0x0080)
#define TMR_DISABLE             (0x0000)
#define TMR_MAX_CNT             (0xffff)

/* defines for tmrBlkRegs timers */
#define TMR_BLK_TMR0		(0)
#define TMR_BLK_WDOG		(1)

/*=====================*
 *  Type defines       *
 *=====================*/

/* Structure of the individual subtimers. */
typedef struct tmrRegs_t 
{
      volatile uint32 loadVal;
      volatile uint32 counter;
      volatile uint32 control;
      uint32          Reserved0;
} tmrRegs;

/* Structure of the overall timer block. */
typedef struct tmrBlkRegs_t 
{
      volatile uint32 status;
      uint32          Reserved1[3];
      tmrRegs       timer[2];
} tmrBlkRegs;

typedef enum tmrMode_t 
{
	FreeRunning,
	Periodic,
	TimeOut,
	WatchDog
} tmrMode;

typedef enum tmrClkFreq_t 
{
	SysClk,
	SysClkDiv4,
	SysClkDiv8,
	SysClkDiv16,
	SysClkDiv32,
	SysClkDiv64,
	SysClkDiv128,
	SysClkDiv256,
	SysClkDiv512,
	SysClkDiv1k,
	SysClkDiv2k,
	SysClkDiv4k,
	SysClkDiv8k,
	SysClkDiv16k,
	SysClkDiv32k,
	SysClkDiv64k,
	MaxSysClkDivVal
} tmrClkFreq;


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void TmrSetDivider ( tmrBlkRegs *tPtr, int timerNum, tmrClkFreq freq );
PUBLIC void TmrSetMode ( tmrBlkRegs *tPtr, int timerNum, tmrMode mode );
PUBLIC void TmrDelay_us ( tmrBlkRegs *tPtr, int timerNum, uint32 microsecs );

PUBLIC uint32 TmrGetMult ( tmrClkFreq freq );


/*=====================*
 *  Macro Functions    *
 *=====================*/

/* FUNCTION_DESC **************************************************************
//
// NAME           TmrEnable()
//
// SYNOPSIS       void TmrEnable ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function enables the timer. The timer will start counting
//                down from the current value in the counter.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define TmrEnable(tPtr,timerNum)	do { tPtr->timer[timerNum].control |= (uint32)TMR_ENABLE; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrDisable()
//
// SYNOPSIS       void TmrDisable ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function will disable the Timer.
//
// NOTE           This function preserves the current counter value. If   
//                the timer is enabled again without being reset, the timer   
//                will start counting down from the previously stopped value. 
//                For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define TmrDisable(tPtr,timerNum)	do { tPtr->timer[timerNum].control &= ~((uint32)TMR_ENABLE); } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrLoad()
//
// SYNOPSIS       void TmrLoad ( tmrBlkRegs *tPtr, int timerNum, 
//                               uint32 val )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//                uint32 val:  counter value.  
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the given Timer load value. It also
//                initializes the timer counter value.
//                If the timer is enabled, it will start counting down from this 
//                value at the configured clock rate, and generate an 
//                interrupt when the counter reaches to zero.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define TmrLoad(tPtr,timerNum,val)	do { tPtr->timer[timerNum].loadVal = val; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrReset()
//
// SYNOPSIS       void TmrReset ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         None
//
// DESCRIPTION    This function resets the given Timer counter value.
//                The timer counter will be reloaded with the timer load value.
//
// NOTE           This function no longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define TmrReset(tPtr,timerNum)   do { if (timerNum==TMR_BLK_TMR0) tPtr->status|=TMR_RESET; if (timerNum==TMR_BLK_WDOG) tPtr->status|=WD_RESET; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrIsEnabled()
//
// SYNOPSIS       bool TmrIsEnabled ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//
// OUTPUT         TRUE/FALSE
//
// DESCRIPTION    This function returns TRUE if the Timer specified by timerNum
//                is enabled.
//
// NOTE           None
//
//                
//
 ******************************************************************************/
#define TmrIsEnabled(tPtr,timerNum)   ( (bool) tPtr->timer[timerNum].control & TMR_ENABLE )


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrIsInt()
//
// SYNOPSIS       bool TmrIsInt ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         TRUE/FALSE
//
// DESCRIPTION    This function returns TRUE if the Timer interrupt has
//                occurred, otherwise it returns FALSE.
//
// NOTE           This function does not clear the interrupt.  
//                No longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define TmrIsInt(tPtr,timerNum)   ( (bool) ((tPtr->status & ((timerNum==TMR_BLK_TMR0)?TMR_INT:WD_INT) ) ? TRUE : FALSE) )


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrClearInt()
//
// SYNOPSIS       void TmrClearInt ( tmrBlkRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: pointer to the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         None
//
// DESCRIPTION    This function clears the Timer interrupt.
//
// NOTE           This function no longer takes a bitmask value, but instead
//                an offset constant (i.e. 0 for timer0, 1 for watchdog).
//
//                Implementation will change with new file conventions.
//
 ******************************************************************************/
#define TmrClearInt(tPtr,timerNum)   do { if (timerNum==TMR_BLK_TMR0) tPtr->status|=TMR_INT; if (timerNum==TMR_BLK_WDOG) tPtr->status|=WD_INT; } while (0)


/* FUNCTION_DESC **************************************************************
//
// NAME           TmrReadCount()
//
// SYNOPSIS       void TmrReadCount ( tmrBlockRegs *tPtr, int timerNum )
//
// TYPE           Inline function
//
// INPUT          tmrBlkRegs *tPtr: base pointer of the timer block
//                int timerNum: number for timer offset within block
//                
// OUTPUT         uint32 count: timer count value
//
// DESCRIPTION    This function returns the current value of the counter.
//
// NOTE           For PalmPak 2.0, timerNum is 0 for timer0, 1 for watchdog.
//
 ******************************************************************************/
#define TmrReadCount(tPtr,timerNum)	( tPtr->timer[timerNum].counter )



#endif /* TIMER_H */


