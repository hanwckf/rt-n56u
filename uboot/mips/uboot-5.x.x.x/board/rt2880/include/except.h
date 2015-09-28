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
*  File Name: except.h
*     Author: Robin Bhagat 
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    10/05/96  RWB   Created.
*    03/24/00  JGU   Moved InstallArmIntVec functionality
*    07/25/00  JGU   Generalized, removing ARM-specific implementation details
*                    and nomenclature.  Moved C_IRQ*Handler routines here from
*                    cpu-specific code.
*    10/25/00  JGU   Moved C_IRQ*Handler routines back to cpu-specific code
*                    due to needing different definitions (but not code)
*                    for ARM mode compilation.
*
*
*******************************************************************************/
/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file contains exception related definitions and 
//    declarations. 
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef EXCEPT_H
#define EXCEPT_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"
#include "intc.h"
#include "cpu_except.h"
#include "cpu.h"


/*=====================*
 *  Defines            *
 *=====================*/

typedef enum intPriority_t {
    NoPriority,
    Priority1,
    Priority2,
    Priority3,
    Priority4,
    Priority5,
    Priority6,
    Priority7,
    Priority8,
    Priority9,
    Priority10,
    Priority11,
    Priority12,
    Priority13,
    Priority14,
    Priority15,
    Priority16
} intPriority;

typedef struct intVector_t {
    uint32      iBit;
    voidFuncPtr iFunc;
} intVector;

typedef struct { intVector irqs[maxInts]; } irqVectorTable;


/*=====================*
 *  External Variables *
 *=====================*/

/* In except.c */
PUBLIC irqVectorTable irqVecTable[];

/* In sysboot.s */
PUBLIC voidFuncPtr exceptVectors[];


/*=====================*
 *  External Functions *
 *=====================*/

/* In except.c */

PUBLIC void RegisterIrqVec ( intCode Code, intPriority Prio,
                             uint32 IrqNum, voidFuncPtr F );
PUBLIC voidFuncPtr InstallNewIrqVec ( uint32 IrqNum, intCode Code,
                                      voidFuncPtr F );
PUBLIC void InitIrqTables ( void );



#endif /* EXCEPT_H */
