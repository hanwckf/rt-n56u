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
*  File Name: pubdefs.h
*     Author: Robin Bhagat 
*
*    Purpose: To define the CPU architecture and compiler independent typedef
*             for portability.
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
*    07/30/96  RWB   Created
*
*
*
*******************************************************************************/

#ifndef PUBDEFS_H
#define PUBDEFS_H

/*=====================*
 *  Include Files      *
 *=====================*/
/* Get INLINE definition. */
#include "compiler.h"            // CPU-specific defs


/*=====================*
 *  Defines            *
 *=====================*/

typedef char              int8;
typedef short             int16;
typedef long              int32;

typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef unsigned long     uint32;
typedef volatile unsigned long     asicreg;

typedef unsigned long     bool;

typedef void (*voidFuncPtr)(void);

#define PUBLIC            extern
#define PRIVATE           static
#define FAST              register
#define REG               register

#ifndef TRUE
#define TRUE              (1)
#define FALSE             (0)
#endif

#define MIN( x, y )	( (x) < (y) ? (x) : (y) )
#define MAX( x, y )	( (x) > (y) ? (x) : (y) )

/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* PUBDEFS_H */
