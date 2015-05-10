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
 *  File Name: regdef.h
 *     Author: Linda Yang
 *
 ******************************************************************************
 *
 * Revision History:
 *
 *      Date    Name  Comments
 *    --------  ---   ------------------------------------
 *    12/22/00  LYT   Created from MIPS sample code.
 *
 *****************************************************************************/


/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains aliases for the MIPS general-purpose registers.
//
// Sp. Notes:
//
******************************************************************************/

#ifndef REGDEF_H
#define REGDEF_H


/*=====================*
 *  Include Files      *
 *=====================*/


/*=====================*
 *  Defines            *
 *=====================*/

#define zero	$0
#define AT	$1
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27
#define gp	$gp
#define sp	$sp
#define s8	$30
#define fp	$30
#define ra	$31
#define pc	$pc


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/



#endif /* REGDEF_H */
