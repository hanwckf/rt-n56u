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
*  File Name: palmpak_tst.h
*     Author: Robin Bhagat 
*
*    Purpose: All the PalmPak ASIC test related definitions, externs, etc.
*
*  Sp. Notes:
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    08/11/97  RWB   Created.
*    09/14/99  MAS   Modified Functions for PalmBeach usage.
*    11/08/00  LRF   Modified functions to use LCD block independent 
*                    functions.
*
*
*
*******************************************************************************/

#ifndef PALMPAK_TST_H
#define PALMPAK_TST_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "lcd.h"
#include "mem_map.h"
#include "pio.h"
#include "pubdefs.h"
#include "sysc.h"


/*=====================*
 *  Defines            *
 *=====================*/

#define TIMEOUT_REG	((uint32)(PALMPAK_BASE + 0xFFFC))
#define timeout(uSec)	(*((uint32 *) TIMEOUT_REG)) = uSec;

#define TEST_PASS	((uint32)0xCAFECAFE)
#define TEST_FAIL	((uint32)0xBADDBEEF)
#define TEST_NA	        ((uint32)0xBAD0CAFE)
#define USE_DATA	((uint32)0x000000FF)


/* Macro to write to test status 2 register */
#define SetTestStat2(val)						\
do									\
{									\
    *(uint32 *)(TEST_RESULT2_REG) = (val);				\
} while (0)


/* These macros are wrappers for functions provided by palmpaklib.
** They will work for both hw simulation and target boards.
** In simulation, the test register will display 0xCAFECAFE for Pass,
** and 0xBADxxynn, where xx is the BlkNum set by SetBlockNumber()
** y is the TstNum set by SetTestNumber(), and nn is the error number.
*/

#define TestFailError(errNum)		Assert(FALSE, (errNum))

#define Assert(cond, code) 						\
{									\
if (!(cond))								\
  {									\
  AssertFunction( code ); /* Call Platform dependent routine */		\
  }									\
}

#define TestPass()		TestEnd(TEST_PASS, "TEST PASS")
#define TestFail()		TestEnd(TEST_FAIL, "TEST FAIL")
#define TestNotApplicable()	TestEnd(TEST_NA, "TEST NOT APPLICABLE")


/*=====================*
 *  External Variables *
 *=====================*/

/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC void AssertFunction( uint32 subcode );
PUBLIC void TestEnd( uint32 teststatus, int8 * msg );
PUBLIC void SetBlockNumber( uint32 BlockNumber );
PUBLIC void SetTestNumber( uint32 TestNumber );
PUBLIC uint8 GetBlockNumber( void );
PUBLIC uint8 GetTestNumber( void );

PUBLIC void LcdClearLineLib( lcdRegs *lcdPtr, uint32 line );
PUBLIC void LcdPuthexLib( lcdRegs *lcdPtr, int8 hexchar );
PUBLIC void LcdPrintStringLib( lcdRegs *lcdPtr, const int8 *thestring );

#endif /* PALMPAK_TST_H */
