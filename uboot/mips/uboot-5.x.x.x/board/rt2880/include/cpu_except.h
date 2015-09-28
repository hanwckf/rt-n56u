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

/******************************************************************************
 *
 *  File Name: cpu_except.h
 *     Author: Linda Yang
 *
 ******************************************************************************
 *
 * Revision History:
 *
 *      Date    Name  Comments
 *    --------  ---   ------------------------------------
 *    12/08/00  LYT   Created.
 *    01/23/01  IST   Deprecated and removed unnecessary functionality.
 *
 *****************************************************************************/


/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains headers for exception routines.
//
// Sp. Notes:
//
// ***************************************************************************/

#ifndef CPU_EXCEPT_H
#define CPU_EXCEPT_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "mem_map.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* 
** If code is in rom, cannot write to the abort handler location.  This means
** that InstallExceptVec() will not work if given the address of the abort
** handler an argument.
** In this case, define the DATA_ABORT_VEC_LOC as the location of the
** bev0 general exception vector, which should always be writable.
** This means that, after InstallExceptVec() is called with DATA_ABORT_VEC_LOC
** as an argument, bev0 general exceptions will only be handled by the newly
** installed abort handler.
** If code is not in rom, the abort handler location is writable and should
** be used, so that the usual general exception processing is done.
*/
#ifdef CODE_IN_ROM
#define DATA_ABORT_VEC_LOC	EXCEPT_VEC_LOC
#else
#define DATA_ABORT_VEC_LOC	(AbortHandlerJump)
#endif


/*=====================*
 *  External Variables *
 *=====================*/
PUBLIC uint32 *AbortHandlerJump;


/*=====================*
 *  External Functions *
 *=====================*/
// This function is not required to be implemented for MIPS, but is used
// in the generic AVTS test files.
#define InstallIrqHandlers()


#endif /* CPU_EXCEPT_H */
