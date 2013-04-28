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
*  File Name: mem_tst.h
*     Author: Linda Yang
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    03/14/00  LYT   Created from functions originally in mac_bufram.c.
*
*
*******************************************************************************/

/* FILE_DESC ******************************************************************/
//
// Purpose:
//    This file has all the definitions, enums, prototype for the different 
//    memory testing functions. 
//
// Sp. Notes:
//
/******************************************************************************/

#ifndef MEM_TST_H
#define MEM_TST_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "pubdefs.h"
#include "product.h"


/*=====================*
 *  Defines            *
 *=====================*/
/* Assert failure codes */
/* Only bits 0-3 of error used so that bits 4-7 can be used as a subTestNum */
#define MEM_ADDR_WRAP_1_FAILCODE		(0x1)
#define MEM_ADDR_WRAP_2_FAILCODE		(0x2)

#define MEM_BAD_WORD_WRITE_READ_FAILCODE	(0x3)
#define MEM_BAD_BYTE_READ_FAILCODE		(0x4)
#define MEM_BAD_HALFWORD_READ_1_FAILCODE	(0x5)
#define MEM_BAD_HALFWORD_READ_2_FAILCODE	(0x6)
#define MEM_BAD_BYTE_WRITE_FAILCODE		(0x7)
#define MEM_BAD_HALFWORD_WRITE_FAILCODE		(0x8)


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
PUBLIC uint32 GetDefaultWrapOffset(uint32 memSize);
PUBLIC uint8 CheckMemAddrWrap(uint32 Addr1, uint32 Addr2);
PUBLIC uint8 CheckMemAccess(uint32 Addr);



#endif /* MEM_TST_H */
