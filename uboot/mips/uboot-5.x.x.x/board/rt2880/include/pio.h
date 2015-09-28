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
*  File Name: pio.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    12/02/97  RWB   ...
*    10/24/00  IST   Merged headers for PalmPak 2.0
*    11/10/00  IST   Updated for PalmPak 2.0 PIO block.
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains the definitions for
//    the programable input/output (PIO) block. 
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef PIO_H
#define PIO_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* PIO Block Specific Defines */
#define PIO_ALL_GPIO_MASK             (0xFFFF)    
#define PIO_GPIO_SHIFT                (0)         

#define PIO_ALL_BTN_MASK              (0x000F)    
#define PIO_BTN_SHIFT                 (0)         

#define PIO_ALL_LED_MASK              (0x000F)    
#define PIO_LED_SHIFT                 (16)         


/* PIO GPIO Pin Definitions */
#define PIO_ALL_GPIO                  (PIO_ALL_GPIO_MASK)
#define PIO_GPIO_0                    (0x0001)
#define PIO_GPIO_1                    (0x0002)
#define PIO_GPIO_2                    (0x0004)
#define PIO_GPIO_3                    (0x0008)
#define PIO_GPIO_4                    (0x0010)
#define PIO_GPIO_5                    (0x0020)
#define PIO_GPIO_6                    (0x0040)
#define PIO_GPIO_7                    (0x0080)
#define PIO_GPIO_8                    (0x0100)
#define PIO_GPIO_9                    (0x0200)
#define PIO_GPIO_10                   (0x0400)
#define PIO_GPIO_11                   (0x0800)
#define PIO_GPIO_12                   (0x1000)
#define PIO_GPIO_13                   (0x2000)
#define PIO_GPIO_14                   (0x4000)
#define PIO_GPIO_15                   (0x8000)

/* Button definitions */
#define PIO_ALL_BTN                   (PIO_ALL_BTN_MASK)
#define PIO_BTN_0                     (0x0001)
#define PIO_BTN_1                     (0x0002)
#define PIO_BTN_2                     (0x0004)
#define PIO_BTN_3                     (0x0008)

/* LED definitions */
#define PIO_ALL_LED                   (PIO_ALL_LED_MASK)
#define PIO_LED_0                     (0x0001)
#define PIO_LED_1                     (0x0002)
#define PIO_LED_2                     (0x0004)
#define PIO_LED_3                     (0x0008)


/* PIO Button Interrupt Status Bit Definitions */
#define PIO_ALL_BTN_INT_STATUS_MASK   (PIO_ALL_BTN_MASK)
#define PIO_BTN_INT_STATUS_SHIFT      (PIO_BTN_SHIFT)

/* PIO Button Interrupt Edge */
#define PIO_ALL_BTN_INT_EDGE_MASK     (PIO_ALL_BTN_MASK)
#define PIO_BTN_INT_EDGE_SHIFT        (PIO_BTN_SHIFT)

/* PIO Button Rise Mask Bit Definitions */
#define PIO_ALL_BTN_RISE_MASK         (PIO_ALL_BTN_MASK)
#define PIO_BTN_RISE_SHIFT            (PIO_BTN_SHIFT)

/* PIO Button Fall Mask Bit Definitions */
#define PIO_ALL_BTN_FALL_MASK         (PIO_ALL_BTN_MASK)
#define PIO_BTN_FALL_SHIFT            (PIO_BTN_SHIFT)

/* PIO LED/Button Data Bit Definitions */
#define PIO_ALL_BTN_DATA_MASK         (PIO_ALL_BTN_MASK)
#define PIO_BTN_DATA_SHIFT            (PIO_BTN_SHIFT)
#define PIO_ALL_LED_DATA_MASK         (PIO_ALL_LED_MASK)
#define PIO_LED_DATA_SHIFT            (PIO_LED_SHIFT)

/* PIO LED/Button Polarity Bit Definitions */
#define PIO_ALL_BTN_POL_MASK          (PIO_ALL_BTN_MASK)
#define PIO_BTN_POL_SHIFT             (PIO_BTN_SHIFT)
#define PIO_ALL_LED_POL_MASK          (PIO_ALL_LED_MASK)
#define PIO_LED_POL_SHIFT             (PIO_LED_SHIFT)

/* PIO LED Config Bit Definitions */
#define PIO_ALL_LED_USE_DATA_MASK     (PIO_ALL_LED_MASK)    /* Led use data */
#define PIO_LED_USE_DATA_SHIFT        (0)
#define PIO_ALL_LED_USE_ALT_MASK      (PIO_ALL_LED_MASK)    /* Led flash */
#define PIO_LED_USE_ALT_SHIFT         (PIO_LED_SHIFT)




/*=====================*
 *  Type Defines       *
 *=====================*/
/* PIO Block Register Structure */   
typedef struct pioRegs_t 
{
	volatile uint32 intStat;	/* 0x00 - Interrupt status */
        volatile uint32 intEdge;        /* Interrupt edge status */
	volatile uint32 rMask;		/* Rising edge */
	volatile uint32 fMask;		/* Falling edge */
	volatile uint32 rsvd0[4];	/* Reserved space */
	volatile uint32 data;		/* 0x20 - Data */
        volatile uint32 direction;      /* Data direction */
        volatile uint32 polarity;       /* Data polarity */
	volatile uint32 rsvd1[5];	/* Reserved space */
	volatile uint32 btnIntStat;	/* 0x40 - Button interrupt status */
        volatile uint32 btnIntEdge;     /* Button interrupt edge */
        volatile uint32 btnRMask;       /* Button rise mask */
	volatile uint32 btnFMask;       /* Button fall mask */
        volatile uint32 rsvd2[4];       /* Reserved space */
	volatile uint32 data2;		/* 0x60 - LED/button data */
	volatile uint32 polarity2;      /* LED/button polarity */
	volatile uint32 ledCfg;         /* LED config */
        volatile uint32 rsvd[33];       /* Reserved space */
        volatile uint32 btnSet;         /* 0xF0 - Hidden button set data */
} pioRegs;



/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void LedTurnOn ( pioRegs *pioBlk, uint32 ledData );
PUBLIC void LedTurnOff ( pioRegs *pioBlk, uint32 ledData );
PUBLIC void LedTurnAllOn ( pioRegs *pioBlk );
PUBLIC void LedTurnAllOff ( pioRegs *pioBlk );
PUBLIC void LedLoadData ( pioRegs *pioBlk, uint32 ledData );


/*=====================*
 *  Macro Functions    *
 *=====================*/


/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetIntStatus()
//                BtnGetIntStatus()
//
// SYNOPSIS       uint32 PioGetIntStatus ( pioRegs *pioBlk )
//                uint32 BtnGetIntStatus ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Interrupt status
//
// DESCRIPTION    These functions return the contents of the Interrupt Status
//                register for either the PIO or the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetIntStatus(pioBlk)     ((pioBlk)->intStat)
#define BtnGetIntStatus(pioBlk)     ((pioBlk)->btnIntStat)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioIsInt()
//                BtnIsInt()
//
// SYNOPSIS       bool PioIsInt ( pioRegs *pioBlk, uint32 sig )
//                bool BtnIsInt ( pioRegs *pioBlk, uint32 sig )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 sig: Contain one or more bits corresponding to the 
//                            requested PIO interrupt signals  
//
// OUTPUT         bool: Sig is set
//
// DESCRIPTION    These functions check if the requested PIO or button 
//                interrupt is set in the appropriate Interrupt Status register.
//
// NOTE           None  
//
/******************************************************************************/
#define PioIsInt(pioBlk,sig)     ( ((pioBlk)->intStat) & (sig) )
#define BtnIsInt(pioBlk,sig) 	 ( ((pioBlk)->btnIntStat) & (sig) )



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearInt()
//                BtnClearInt()
//
// SYNOPSIS       void PioClearInt ( pioRegs *pioBlk, uint32 sig )
//                void BtnClearInt ( pioRegs *pioBlk, uint32 sig )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 sig: Contain one or more bits corresponding to the 
//                            requested PIO interrupt signal  
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested PIO or button interrupt.
//
// NOTE           None  
//
/******************************************************************************/
#define PioClearInt(pioBlk,sig)  	                                    \
  do                                                                        \
  {                                                                         \
    (pioBlk)->intStat = (sig);                                              \
  } while (0)

#define BtnClearInt(pioBlk,sig)  	                                    \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnIntStat = (sig);                                           \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetIntEdgeStatus()
//                BtnGetIntEdgeStatus()
//
// SYNOPSIS       uint32 PioGetIntEdgeStatus ( pioRegs *pioBlk )
//                uint32 BtnGetIntEdgeStatus ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Interrupt Edge status
//
// DESCRIPTION    These functions return the contents of the Interrupt Edge 
//                Status register for the PIO block and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetIntEdgeStatus(pioBlk)     ((pioBlk)->intEdge)
#define BtnGetIntEdgeStatus(pioBlk)     ((pioBlk)->btnIntEdge)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioIsIntEdge()
//                BtnIsIntEdge()
//
// SYNOPSIS       bool PioIsIntEdge ( pioRegs *pioBlk, uint32 sig )
//                bool BtnIsIntEdge ( pioRegs *pioBlk, uint32 sig )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 sig: Contain one or more bits corresponding to the 
//                            requested PIO interrupt edge signals  
//
// OUTPUT         bool: Interrupt Edge is set
//
// DESCRIPTION    These functions check if the requested PIO or button 
//                interrupt is set in the appropriate Interrupt Edge Status 
//                register.
//
// NOTE           None  
//
/******************************************************************************/
#define PioIsIntEdge(pioBlk,sig)     ( ((pioBlk)->intEdge) & (sig) )
#define BtnIsIntEdge(pioBlk,sig)     ( ((pioBlk)->btnIntEdge) & (sig) )



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearIntEdge()
//                BtnClearIntEdge()
//
// SYNOPSIS       void PioClearIntEdge ( pioRegs *pioBlk, uint32 sig )
//                void BtnClearIntEdge ( pioRegs *pioBlk, uint32 sig )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 sig: Contain one or more bits corresponding to the 
//                            requested PIO interrupt edge signal  
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested PIO or button edge
//                interrupt.
//
// NOTE           None  
//
/******************************************************************************/
#define PioClearIntEdge(pioBlk,sig)  	                                    \
  do                                                                        \
  {                                                                         \
    (pioBlk)->intEdge = (sig);                                              \
  } while (0)

#define BtnClearIntEdge(pioBlk,sig)  	                                    \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnIntEdge = (sig);                                           \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetRiseMask()
//                BtnGetRiseMask()
//
// SYNOPSIS       uint32 PioGetRiseMask ( pioRegs *pioBlk )
//                uint32 BtnGetRiseMask ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Rise mask
//
// DESCRIPTION    These functions return the contents of the Rise Mask register
//                for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetRiseMask(pioBlk)                                              \
    ( (((pioBlk)->rMask) >> PIO_GPIO_SHIFT) & PIO_ALL_GPIO_MASK )

#define BtnGetRiseMask(pioBlk)                                              \
    ( (((pioBlk)->btnRMask) >> PIO_BTN_RISE_SHIFT) & PIO_ALL_BTN_RISE_MASK )



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetRiseMask()
//                BtnSetRiseMask()
//
// SYNOPSIS       void PioSetRiseMask ( pioRegs *pioBlk, uint32 data )
//                void BtnSetRiseMask ( pioRegs *pioBlk, uint32 data )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 data: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    These functions set the requested bits in the Rise Mask 
//                register for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioSetRiseMask(pioBlk,data)                                         \
  do                                                                        \
  {                                                                         \
    (pioBlk)->rMask |= ( ((uint32)((data) & PIO_ALL_GPIO_MASK)) <<          \
                        PIO_GPIO_SHIFT );                                   \
  } while (0)

#define BtnSetRiseMask(pioBlk,data)                                         \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnRMask |= ( ((uint32)((data) & PIO_ALL_BTN_RISE_MASK)) <<   \
                        PIO_BTN_RISE_SHIFT );                               \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearRiseMask()
//                BtnClearRiseMask()
//
// SYNOPSIS       void PioClearRiseMask ( pioRegs *pioBlk, uint32 data )
//                void BtnClearRiseMask ( pioRegs *pioBlk, uint32 data )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 data: bits to be cleared
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested bits in the RiseMask 
//                register for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioClearRiseMask(pioBlk,data)                                       \
  do                                                                        \
  {                                                                         \
    (pioBlk)->rMask &= ( ~((uint32)((data) & PIO_ALL_GPIO_MASK)) <<         \
                         PIO_GPIO_SHIFT );                                  \
  } while (0)

#define BtnClearRiseMask(pioBlk,data)                                       \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnRMask &= ( ~((uint32)((data) & PIO_ALL_BTN_RISE_MASK)) <<  \
                         PIO_BTN_RISE_SHIFT );                              \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetFallMask()
//                BtnGetFallMask()
//
// SYNOPSIS       uint32 PioGetFallMask ( pioRegs *pioBlk )
//                uint32 BtnGetFallMask ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Fall mask
//
// DESCRIPTION    These functions return the contents of the Fall Mask register
//                for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetFallMask(pioBlk)                                              \
    ( (((pioBlk)->fMask) >> PIO_GPIO_SHIFT) & PIO_ALL_GPIO_MASK )

#define BtnGetFallMask(pioBlk)                                              \
    ( (((pioBlk)->btnFMask) >> PIO_BTN_FALL_SHIFT) & PIO_ALL_BTN_FALL_MASK )



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetFallMask()
//                BtnSetFallMask()
//
// SYNOPSIS       void PioSetFallMask ( pioRegs *pioBlk, uint32 data )
//                void BtnSetFallMask ( pioRegs *pioBlk, uint32 data )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 data: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    These functions set the requested bits in the Fall Mask 
//                register for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioSetFallMask(pioBlk,data)                                         \
  do                                                                        \
  {                                                                         \
    (pioBlk)->fMask |= ( ((uint32)((data) & PIO_ALL_GPIO_MASK)) <<          \
                        PIO_GPIO_SHIFT );                                   \
  } while (0)

#define BtnSetFallMask(pioBlk,data)                                         \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnFMask |= ( ((uint32)((data) & PIO_ALL_BTN_FALL_MASK)) <<   \
                        PIO_BTN_FALL_SHIFT );                               \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearFallMask()
//                BtnClearFallMask()
//
// SYNOPSIS       void PioClearFallMask ( pioRegs *pioBlk, uint32 data )
//                void BtnClearFallMask ( pioRegs *pioBlk, uint32 data )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 data: bits to be cleared
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested bits in the FallMask 
//                register for the PIO and the buttons.
//
// NOTE           None  
//
/******************************************************************************/
#define PioClearFallMask(pioBlk,data)                                       \
  do                                                                        \
  {                                                                         \
    (pioBlk)->fMask &= ( ~((uint32)((data) & PIO_ALL_GPIO_MASK)) <<         \
                         PIO_GPIO_SHIFT );                                  \
  } while (0)

#define BtnClearFallMask(pioBlk,data)                                       \
  do                                                                        \
  {                                                                         \
    (pioBlk)->btnFMask &= ( ~((uint32)((data) & PIO_ALL_BTN_FALL_MASK)) <<  \
                         PIO_BTN_FALL_SHIFT );                              \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetData()
//                BtnGetData()
//                LedGetData()
//
// SYNOPSIS       uint32 PioGetData ( pioRegs *pioBlk )
//                uint32 BtnGetData ( pioRegs *pioBlk )
//                uint32 LedGetData ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Data
//
// DESCRIPTION    These functions return the contents of the Data register
//                for the PIO, buttons, and the LEDs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetData(pioBlk)                                                  \
    ( (((pioBlk)->data) >> PIO_GPIO_SHIFT) & PIO_ALL_GPIO_MASK )

#define BtnGetData(pioBlk)                                                  \
    ( (((pioBlk)->data2) >> PIO_BTN_DATA_SHIFT) & PIO_ALL_BTN_DATA_MASK )

#define LedGetData(pioBlk)                                                  \
    ( (((pioBlk)->data2) >> PIO_LED_DATA_SHIFT) & PIO_ALL_LED_DATA_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetData()
//                LedSetData()
//
// SYNOPSIS       void PioSetData ( pioRegs *pioBlk, uint32 bits )
//                void LedSetData ( pioRegs *pioBlk, uint32 bits )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 bits: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    These functions set the requested bits in the Data register
//                for the PIO and LEDs.
//
// NOTE           No other bits are affected.
//
/******************************************************************************/
#define PioSetData(pioBlk,bits)                                             \
  do                                                                        \
  {                                                                         \
    (pioBlk)->data |= ( ((uint32)(bits) & PIO_ALL_GPIO_MASK) <<             \
                        PIO_GPIO_SHIFT );                                   \
  } while (0)

#define LedSetData(pioBlk,bits)                                             \
  do                                                                        \
  {                                                                         \
    (pioBlk)->data2 |= ( ((uint32)(bits) & PIO_ALL_LED_MASK) <<             \
                        PIO_LED_SHIFT );                                    \
  } while (0)

 

/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearData()
//                LedClearData()
//
// SYNOPSIS       void PioClearData ( pioRegs *pioBlk, uint32 bits )
//                void LedClearData ( pioRegs *pioBlk, uint32 bits )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 bits: bits to be cleared
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested bits in the Data register
//                for the PIO and LEDs.
//
// NOTE           No other bits are affected.
//
/******************************************************************************/
#define PioClearData(pioBlk,bits)                                            \
  do                                                                         \
  {                                                                          \
    (pioBlk)->data &= ( ~((uint32)(bits) & PIO_ALL_GPIO_MASK) <<             \
                         PIO_GPIO_SHIFT );                                   \
  } while (0)

#define LedClearData(pioBlk,bits)                                            \
  do                                                                         \
  {                                                                          \
    (pioBlk)->data2 &= ( ~(((uint32)(bits) & PIO_ALL_LED_MASK) <<            \
                        PIO_LED_SHIFT) );                                    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioPutData()
//                LedPutData()
//
// SYNOPSIS       void PioPutData ( pioRegs *pioBlk, uint32 bits )
//                void LedPutData ( pioRegs *pioBlk, uint32 bits )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 bits: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    These functions set the requested bits in the Data register
//                (for the PIO and LEDs), and clear the remaining bits.  
//
// NOTE           None 
//
/******************************************************************************/
#define PioPutData(pioBlk,bits)                                              \
  do                                                                         \
  {                                                                          \
    (pioBlk)->data = ( ((uint32)(bits) & PIO_ALL_GPIO_MASK) <<               \
                         PIO_GPIO_SHIFT );                                   \
  } while (0)

#define LedPutData(pioBlk,bits)                                              \
  do                                                                         \
  {                                                                          \
    (pioBlk)->data2 &= ~(PIO_ALL_LED_MASK << PIO_LED_SHIFT);                 \
    (pioBlk)->data2 |= ((bits) & PIO_ALL_LED_MASK) << PIO_LED_SHIFT;         \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           BtnPutData()
//
// SYNOPSIS       void BtnPutData ( pioRegs *pioBlk, uint32 bits )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 bits: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    This function simulates the buttons being physically 
//                pressed.  It is used for testing purposes to generate
//                interrupts.
//
// NOTE           Not in the spec, as it is not necessary for real apps.
//
/******************************************************************************/
#define BtnPutData(pioBlk,bits)                                              \
  do                                                                         \
  {                                                                          \
    (pioBlk)->btnSet = ((bits) & PIO_ALL_BTN_DATA_MASK) << PIO_BTN_SHIFT;    \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetDir()
//
// SYNOPSIS       uint32 PioGetDir ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Direction
//
// DESCRIPTION    This function returns the contents of the Direction register.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetDir(pioBlk)                                                  \
    ( (((pioBlk)->direction) >> PIO_GPIO_SHIFT) & PIO_ALL_GPIO_MASK )



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetDirOutput ()
//
// SYNOPSIS       void PioSetDirOutput ( pioRegs *pioBlk, uint32 dir )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 dir: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the requested PIO pins to outputs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioSetDirOutput(pioBlk,dir)                                         \
  do                                                                        \
  {                                                                         \
    (pioBlk)->direction |= ( ((uint32)(dir) & PIO_ALL_GPIO_MASK) <<         \
                             PIO_GPIO_SHIFT );                              \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetDirInput ()
//
// SYNOPSIS       void PioSetDirInput ( pioRegs *pioBlk, uint32 Dir )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 dir: bits to be set to inputs
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the requested PIO pins to inputs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioSetDirInput(pioBlk,dir)                                          \
  do                                                                        \
  {                                                                         \
    (pioBlk)->direction &= ( ~((uint32)(dir) & PIO_ALL_GPIO_MASK) <<        \
                              PIO_GPIO_SHIFT );                             \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioGetPol()
//                BtnGetPol()
//                LedGetPol()
//
// SYNOPSIS       uint32 PioGetPol ( pioRegs *pioBlk )
//                uint32 BtnGetPol ( pioRegs *pioBlk )
//                uint32 LedGetPol ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32: Polarity
//
// DESCRIPTION    These functions return the contents of the Polarity register
//                for the PIO, buttons, and LEDs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioGetPol(pioBlk)                                                  \
    ( (((pioBlk)->polarity) >> PIO_GPIO_SHIFT) & PIO_ALL_GPIO_MASK )

#define BtnGetPol(pioBlk)                                                  \
    ( (((pioBlk)->polarity2) >> PIO_BTN_POL_SHIFT) & PIO_ALL_BTN_POL_MASK )

#define LedGetPol(pioBlk)                                                  \
  ( (((pioBlk)->polarity2) >> PIO_LED_POL_SHIFT) & PIO_ALL_LED_POL_MASK )




/* FUNCTION_DESC **************************************************************/
//
// NAME           PioSetPol()
//                BtnSetPol()
//                LedSetPol()
//
// SYNOPSIS       void PioSetPol ( pioRegs *pioBlk, uint32 pol )
//                void BtnSetPol ( pioRegs *pioBlk, uint32 pol )
//                void LedSetPol ( pioRegs *pioBlk, uint32 pol )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 pol: bits to be set
//
// OUTPUT         None
//
// DESCRIPTION    These functions set the requested bits in the Polarity
//                register for the PIO, buttons, and LEDs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioSetPol(pioBlk,pol)                                               \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity |= ( ((uint32)((pol) & PIO_ALL_GPIO_MASK)) <<        \
                             PIO_GPIO_SHIFT );                              \
  } while (0)

#define BtnSetPol(pioBlk,pol)                                               \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity2 |= ( ((uint32)((pol) & PIO_ALL_BTN_POL_MASK)) <<    \
                             PIO_BTN_POL_SHIFT );                           \
  } while (0)

#define LedSetPol(pioBlk,pol)                                               \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity2 |= ( ((uint32)((pol) & PIO_ALL_LED_POL_MASK)) <<    \
                             PIO_LED_POL_SHIFT );                           \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           PioClearPol()
//                BtnClearPol()
//                LedClearPol()
//
// SYNOPSIS       void PioClearPol ( pioRegs *pioBlk, uint32 pol )
//                void BtnClearPol ( pioRegs *pioBlk, uint32 pol )
//                void LedClearPol ( pioRegs *pioBlk, uint32 pol )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 pol: bits to be cleared
//
// OUTPUT         None
//
// DESCRIPTION    These functions clear the requested bits in the Polarity 
//                register for the PIO, buttons, and LEDs.
//
// NOTE           None  
//
/******************************************************************************/
#define PioClearPol(pioBlk,pol)                                             \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity &= ( ~((uint32)((pol) & PIO_ALL_GPIO_MASK)) <<       \
                              PIO_GPIO_SHIFT );                             \
  } while (0)

#define BtnClearPol(pioBlk,pol)                                             \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity2 &= ( ~((uint32)((pol) & PIO_ALL_BTN_POL_MASK)) <<   \
                              PIO_BTN_POL_SHIFT );                          \
  } while (0)

#define LedClearPol(pioBlk,pol)                                             \
  do                                                                        \
  {                                                                         \
    (pioBlk)->polarity2 &= ( ~((uint32)((pol) & PIO_ALL_LED_POL_MASK)) <<   \
                              PIO_LED_POL_SHIFT );                          \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LedGetDatCfg()
//
// SYNOPSIS       void LedGetDatCfg ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32 useDatConfig
//
// DESCRIPTION    This function returns the UseDat section of the LED config
//                register.  A 1 indicates the LED gets input from the data
//                register, and a 0 indicates to use input from the buttons.
//
// NOTE           None
//
/******************************************************************************/
#define LedGetDatCfg(pioBlk)                                                \
    ( ((pioBlk)->ledCfg >> PIO_LED_USE_DATA_SHIFT) & PIO_ALL_LED_USE_DATA_MASK )



/* FUNCTION_DESC **************************************************************/
//
// NAME           LedSetDatCfg()
//
// SYNOPSIS       void LedSetDatCfg ( pioRegs *pioBlk, uint32 datCfg )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 datCfg: new useDat configuration
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the UseDat section of the LED config
//                register.  A 1 indicates the LED gets input from the data
//                register, and a 0 indicates to use input from the buttons.
//
// NOTE           None
//
/******************************************************************************/
#define LedSetDatCfg(pioBlk,datCfg)                                          \
    (pioBlk)->ledCfg |= ((uint32)((datCfg) & PIO_ALL_LED_USE_DATA_MASK) <<   \
                     PIO_LED_USE_DATA_SHIFT);  


/* FUNCTION_DESC **************************************************************/
//
// NAME           LedClearDatCfg()
//
// SYNOPSIS       void LedClearDatCfg ( pioRegs *pioBlk, uint32 datCfg )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 datCfg: new useDat configuration
//
// OUTPUT         None
//
// DESCRIPTION    This function clears the UseDat section of the LED config
//                register.  A 1 indicates the LEDs gets input from the data
//                register, and a 0 indicates to use input from the buttons.
//
// NOTE           None
//
/******************************************************************************/
#define LedClearDatCfg(pioBlk,datCfg)                                        \
    (pioBlk)->ledCfg &= ~(((datCfg) & PIO_ALL_LED_USE_DAT_MASK) <<           \
                         PIO_LED_USE_DAT_SHIFT);



/* FUNCTION_DESC **************************************************************/
//
// NAME           LedGetAltCfg()
//
// SYNOPSIS       void LedGetAltCfg ( pioRegs *pioBlk )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//
// OUTPUT         uint32 useAltConfig
//
// DESCRIPTION    This function returns the UseAlt section of the LED config
//                register.  A 1 indicates the LEDs use the alternate behavior
//                (i.e. flash) and a 0 indicates normal behavior.
//
// NOTE           None
//
/******************************************************************************/
#define LedGetAltCfg(pioBlk)                                               \
    ( ((pioBlk)->ledCfg >> PIO_LED_USE_ALT_SHIFT) & PIO_ALL_LED_USE_ALT_MASK )



/* FUNCTION_DESC **************************************************************/
//
// NAME           LedSetAltCfg()
//
// SYNOPSIS       void LedSetAltCfg ( pioRegs *pioBlk, uint32 altCfg )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 altCfg: new useAlt configuration
//
// OUTPUT         None
//
// DESCRIPTION    This function sets the UseAlt section of the LED config
//                register.  A 1 indicates the LEDs use the alternate behavior
//                (i.e. flash) and a 0 indicates normal behavior.
//
// NOTE           None
//
/******************************************************************************/
#define LedSetAltCfg(pioBlk,altCfg)                                          \
  do                                                                         \
  {                                                                          \
    (pioBlk)->ledCfg &= ~(PIO_ALL_LED_USE_ALT_MASK << PIO_LED_USE_ALT_SHIFT);\
    (pioBlk)->ledCfg |= (uint32)(((altCfg) & PIO_ALL_LED_USE_ALT_MASK)) <<   \
                     PIO_LED_USE_ALT_SHIFT;                                  \
  } while (0)



/* FUNCTION_DESC **************************************************************/
//
// NAME           LedClearAltCfg()
//
// SYNOPSIS       void LedClearAltCfg ( pioRegs *pioBlk, uint32 altCfg )
//
// TYPE           Macro
//
// INPUT          pioRegs *pioBlk: PIO block pointer
//                uint32 altCfg: new useAlt configuration
//
// OUTPUT         None
//
// DESCRIPTION    This function clears the UseAlt section of the LED config
//                register.  A 1 indicates the LEDs use the alternate behavior
//                (i.e. flash) and a 0 indicates normal behavior.
//
// NOTE           None
//
/******************************************************************************/
#define LedClearAltCfg(pioBlk,altCfg)                                        \
    (pioBlk)->ledCfg &= ~(((altCfg) & PIO_ALL_LED_USE_ALT_MASK) <<           \
                         PIO_LED_USE_ALT_SHIFT);



#endif /* PIO_H */




