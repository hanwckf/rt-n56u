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
 *  File Name: CpuCp0RegDef.h
 *     Author: Linda Yang
 *
 ******************************************************************************
 *
 * Revision History:
 *
 *      Date    Name  Comments
 *    --------  ---   ------------------------------------
 *    12/22/00  LYT   Created from MIPS sample file.
 *
 *****************************************************************************/


/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file contains the definitions and macros for the MIPS 4k 
//    Coprocessor 0 register set.
//
// Sp. Notes:
//    This file is a slightly modified version of a MIPS sample file.
//
******************************************************************************/

#ifndef CPUCP0REGDEF_H
#define CPUCP0REGDEF_H

/*=====================*
 *  Include Files      *
 *=====================*/


/*=====================*
 *  Defines            *
 *=====================*/


/*
 *************************************************************************
 *                H A R D W A R E   G P R   I N D I C E S                *
 *************************************************************************
 *
 * These definitions provide the index (number) of the GPR, as opposed
 * to the assembler register name ($n).
 */

#define R_r0			 0
#define R_r1			 1
#define R_r2			 2
#define R_r3			 3
#define R_r4			 4
#define R_r5			 5
#define R_r6			 6
#define R_r7			 7
#define R_r8			 8
#define R_r9			 9
#define R_r10			10
#define R_r11			11
#define R_r12			12
#define R_r13			13
#define R_r14			14
#define R_r15			15
#define R_r16			16
#define R_r17			17
#define R_r18			18
#define R_r19			19
#define R_r20			20
#define R_r21			21
#define R_r22			22
#define R_r23			23
#define R_r24			24
#define R_r25			25
#define R_r26			26
#define R_r27			27
#define R_r28			28
#define R_r29			29
#define R_r30			30
#define R_r31			31
#define R_hi			32			/* Hi register */
#define R_lo			33			/* Lo register */


/*
 *************************************************************************
 *             C P 0   R E G I S T E R   D E F I N I T I O N S           *
 *************************************************************************
 * Each register has the following definitions:
 *
 *	C0_rrr		The register number (as a $n value)
 *	R_C0_rrr	The register index (as an integer corresponding
 *			to the register number)
 *
 * Each field in a register has the following definitions:
 *
 *	S_rrrfff	The shift count required to right-justify
 *			the field.  This corresponds to the bit
 *			number of the right-most bit in the field.
 *	M_rrrfff	The Mask required to isolate the field.
 *
 *	r		Denotes reserved bits in the register.
 *	0		Denotes bits which must be written as 0,
 *			and returns 0 on reads.
 *
 * Register diagrams included below as comments correspond to the
 * MIPS32 and MIPS64 architecture specifications.  Refer to other
 * sources for register diagrams for older architectures.
 *
 *      K_rrr           The possible values for fields in a register
 *                      (LYT 12-19-00)
 */


/*
 ************************************************************************
 *                 I N D E X   R E G I S T E R   ( 0 )                  *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |P|                         0                           | Index | Index
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 4K Core implements a 16 entry TLB, so Index is 4 bits wide. (LYT 12-19-00)
 */

#define C0_Index		$0
#define R_C0_Index		0

#define S_IndexP		31			/* Probe failure (R)*/
#define M_IndexP		(0x1 << S_IndexP)

#define S_IndexIndex		0			/* TLB index (R/W)*/
#define M_IndexIndex		(0xf << S_IndexIndex)

#define M_Index0Fields		0x7ffffff0		/* 0 bits */
#define M_IndexRFields		0x80000000		/* Read-only bits */


/*
 ************************************************************************
 *                R A N D O M   R E G I S T E R   ( 1 )                 *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            0                          | Index | Random
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 4K Core implements a 16 entry TLB, so Index is 4 bits wide. (LYT 12-19-00)
 */

#define C0_Random		$1
#define R_C0_Random		1

#define S_RandomIndex		0			/* TLB random index (R)*/
#define M_RandomIndex		(0xf << S_RandomIndex)

#define M_Random0Fields		0xfffffff0
#define M_RandomRFields		0x0000000f


/*
 ************************************************************************
 *              E N T R Y L O 0   R E G I S T E R   ( 2 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | r |   0   |                 PFN                   |  C  |D|V|G| EntryLo0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_EntryLo0		$2
#define R_C0_EntryLo0		2

#define S_EntryLoPFN		6			/* PFN (R/W) */
#define M_EntryLoPFN		(0xfffff << S_EntryLoPFN)
#define S_EntryLoC		3			/* Coherency attribute (R/W) */
#define M_EntryLoC		(0x7 << S_EntryLoC)
#define S_EntryLoD		2			/* Dirty (R/W) */
#define M_EntryLoD		(0x1 << S_EntryLoD)
#define S_EntryLoV		1			/* Valid (R/W) */
#define M_EntryLoV		(0x1 << S_EntryLoV)
#define S_EntryLoG		0			/* Global (R/W) */
#define M_EntryLoG		(0x1 << S_EntryLoG)

#define M_EntryLoOddPFN		(0x1 << S_EntryLoPFN)	/* Odd PFN bit */
// CHECKME:  What are next two defines used for?  (LYT 12-19-00)
// UNDEFINED:  K_PageAlign (LYT 12-19-00)
#define S_EntryLo_LS		S_EntryLoPFN		/* Position PFN to appropriate position */
#define S_EntryLo_RS		K_PageAlign		/* Right-justify PFN */

#define M_EntryLo0Fields	0x3c000000
#define M_EntryLoRFields	0xc0000000

/*
 * Cache attribute values in the C field of EntryLo and the
 * K* fields of Config
 */
/* For MIPS32 architecture, 2 and 3 are used.
 * Values 0, 1, 4, 5, 6 are not used and are mapped to 3.
 * Value 7 is not used and is mapped to 2. (LYT 12-19-00)
 */
#define K_CacheAttrCWTnWA	0			/* Cacheable, write-thru, no write allocate */
#define K_CacheAttrCWTWA	1			/* Cacheable, write-thru, write allocate */
#define K_CacheAttrU		2			/* Uncached */
#define K_CacheAttrC		3			/* Cacheable */
#define K_CacheAttrCN		3			/* Cacheable, non-coherent */
#define K_CacheAttrCCE		4			/* Cacheable, coherent, exclusive */
#define K_CacheAttrCCS		5			/* Cacheable, coherent, shared */
#define K_CacheAttrCCU		6			/* Cacheable, coherent, update */
#define K_CacheAttrUA		7			/* Uncached accelerated */


/*
 ************************************************************************
 *              E N T R Y L O 1   R E G I S T E R   ( 3 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | r |   0   |                 PFN                   |  C  |D|V|G| EntryLo1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_EntryLo1		$3
#define R_C0_EntryLo1		3

/*
 * Field definitions are as given for EntryLo0 above
 */


/*
 ************************************************************************
 *               C O N T E X T   R E G I S T E R   ( 4 )                *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      PTEBase    |            BadVPN<31:13>            |   0   | Context
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Context		$4
#define R_C0_Context		4

#define S_ContextPTEBase	23			/* PTE base (R/W) */
#define M_ContextPTEBase	(0x1ff << S_ContextPTEBase)
#define S_ContextBadVPN		4			/* BadVPN2 (R) */
#define M_ContextBadVPN		(0x7ffff << S_ContextBadVPN)

#define S_ContextBadVPN_LS	9			/* Position BadVPN to bit 31 */
#define S_ContextBadVPN_RS	13			/* Right-justify shifted BadVPN field */

#define M_Context0Fields	0x0000000f
#define M_ContextRFields	0x007ffff0


/*
 ************************************************************************
 *              P A G E M A S K   R E G I S T E R   ( 5 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |      0      |        Mask           |            0            | PageMask
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_PageMask		$5
#define R_C0_PageMask		5

#define S_PageMaskMask		13			/* Mask (R/W) */
#define M_PageMaskMask		(0xfff << S_PageMaskMask)

#define M_PageMask0Fields	0xfe001fff
#define M_PageMaskRFields	0x00000000

/*
 * Values in the Mask field
 */
#define K_PageMask4K		0x000			/* K_PageMasknn values are values for use */
#define K_PageMask16K		0x003			/* with KReqPageAttributes or KReqPageMask macros */
#define K_PageMask64K		0x00f
#define K_PageMask256K		0x03f
#define K_PageMask1M		0x0ff
#define K_PageMask4M		0x3ff
#define K_PageMask16M		0xfff

#define M_PageMask4K		(K_PageMask4K << S_PageMaskMask)  /* M_PageMasknn values are masks */
#define M_PageMask16K		(K_PageMask16K << S_PageMaskMask) /* in position in the PageMask register */
#define M_PageMask64K		(K_PageMask64K << S_PageMaskMask)
#define M_PageMask256K		(K_PageMask256K << S_PageMaskMask)
#define M_PageMask1M		(K_PageMask1M << S_PageMaskMask)
#define M_PageMask4M		(K_PageMask4M << S_PageMaskMask)
#define M_PageMask16M		(K_PageMask16M << S_PageMaskMask)


/*
 ************************************************************************
 *                 W I R E D   R E G I S T E R   ( 6 )                  *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            0                          | Index | Wired
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * 4K Core implements a 16 entry TLB, so Index is 4 bits wide. (LYT 12-19-00)
 */

#define C0_Wired		$6
#define R_C0_Wired		6

#define S_WiredIndex		0			/* TLB wired boundary (R/W) */
#define M_WiredIndex		(0xf << S_WiredIndex)

#define M_Wired0Fields		0xfffffff0
#define M_WiredRFields		0x00000000


/*
 ************************************************************************
 *              B A D V A D D R   R E G I S T E R   ( 8 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                    Bad Virtual Address                        | BadVAddr
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_BadVAddr		$8			/* (R) */
#define R_C0_BadVAddr		8

// UNDEFINED:  K_PageSize (LYT 12-19-00)
#define M_BadVAddrOddPage	K_PageSize		/* Even/Odd VA bit for pair of PAs */

#define M_BadVAddr0Fields	0x00000000
#define M_BadVAddrRFields	0xffffffff


/*
 ************************************************************************
 *                 C O U N T   R E G I S T E R   ( 9 )                  *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Count Value                           | Count
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Count		$9			/* (R/W) */
#define R_C0_Count		9

#define M_Count0Fields		0x00000000
#define M_CountRFields		0x00000000


/*
 ************************************************************************
 *              E N T R Y H I   R E G I S T E R   ( 1 0 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                VPN2                 |    0    |     ASID      | EntryHi
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_EntryHi		$10
#define R_C0_EntryHi		10

#define S_EntryHiVPN2		13			/* VPN/2 (R/W) */
#define M_EntryHiVPN2		(0x7ffff << S_EntryHiVPN2)
#define S_EntryHiASID		0			/* ASID (R/W) */
#define M_EntryHiASID		(0xff << S_EntryHiASID)

// CHECKME:  What is S_EntryHiVPN_Shf used for? (LYT 12-19-00)
#define S_EntryHiVPN_Shf	S_EntryHiVPN2

#define M_EntryHi0Fields	0x00001f00
#define M_EntryHiRFields	0x00000000


/*
 ************************************************************************
 *              C O M P A R E   R E G I S T E R   ( 1 1 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Compare Value                          | Compare
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Compare		$11			/* (R/W) */
#define R_C0_Compare		11

#define M_Compare0Fields	0x00000000
#define M_CompareRFields	0x00000000


/*
 ************************************************************************
 *               S T A T U S   R E G I S T E R   ( 1 2 )                *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |C|C|C|C|R| |R|   |B|T|S|N| |   |I|I|I|I|I|I|I|I| | | |U| |E|E|I|
 * |U|U|U|U|P|r|E| 0 |E|S|R|M|0| r |M|M|M|M|M|M|M|M|r|r|r|M|r|R|X|E| Status
 * |3|2|1|0| | | |   |V| | |I| |   |7|6|5|4|3|2|1|0| | | | | |L|L| |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Status		$12
#define R_C0_Status		12

#define S_StatusCU		28			/* Coprocessor enable (R/W) */
#define M_StatusCU		(0xf << S_StatusCU)
#define S_StatusCU3		31
#define M_StatusCU3     	(0x1 << S_StatusCU3)
#define S_StatusCU2		30
#define M_StatusCU2		(0x1 << S_StatusCU2)
#define S_StatusCU1		29
#define M_StatusCU1		(0x1 << S_StatusCU1)
#define S_StatusCU0		28
#define M_StatusCU0		(0x1 << S_StatusCU0)
#define S_StatusRP		27			/* Enable reduced power mode (R/W) */
#define M_StatusRP		(0x1 << S_StatusRP)
#define S_StatusRE		25			/* Enable reverse endian (R/W) */
#define M_StatusRE		(0x1 << S_StatusRE)
#define S_StatusBEV		22			/* Enable Boot Exception Vectors (R/W) */
#define M_StatusBEV		(0x1 << S_StatusBEV)
#define S_StatusTS		21			/* Denote TLB shutdown (R/W) */
#define M_StatusTS		(0x1 << S_StatusTS)
#define S_StatusSR		20			/* Denote soft reset (R/W) */
#define M_StatusSR		(0x1 << S_StatusSR)
#define S_StatusNMI		19			/* Denote NMI (R/W) */
#define M_StatusNMI		(0x1 << S_StatusNMI)
#define S_StatusIM		8			/* Interrupt mask (R/W) */
#define M_StatusIM		(0xff << S_StatusIM)
#define S_StatusIM7		15
#define M_StatusIM7		(0x1 << S_StatusIM7)
#define S_StatusIM6		14
#define M_StatusIM6		(0x1 << S_StatusIM6)
#define S_StatusIM5		13
#define M_StatusIM5		(0x1 << S_StatusIM5)
#define S_StatusIM4		12
#define M_StatusIM4		(0x1 << S_StatusIM4)
#define S_StatusIM3		11
#define M_StatusIM3		(0x1 << S_StatusIM3)
#define S_StatusIM2		10
#define M_StatusIM2		(0x1 << S_StatusIM2)
#define S_StatusIM1		9
#define M_StatusIM1		(0x1 << S_StatusIM1)
#define S_StatusIM0		8
#define M_StatusIM0		(0x1 << S_StatusIM0)
#define S_StatusUM		4			/* User mode if supervisor mode not implemented (R/W) */
#define M_StatusUM		(0x1 << S_StatusUM)
#define S_StatusERL		2			/* Denotes error level (R/W) */
#define M_StatusERL		(0x1 << S_StatusERL)
#define S_StatusEXL		1			/* Denotes exception level (R/W) */
#define M_StatusEXL		(0x1 << S_StatusEXL)
#define S_StatusIE		0			/* Enables interrupts (R/W) */
#define M_StatusIE		(0x1 << S_StatusIE)

#define M_Status0Fields		0x01840000
#define M_StatusRFields		0x040300e8


/*
 ************************************************************************
 *                C A U S E   R E G I S T E R   ( 1 3 )                 *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |B| | C |       |I|W|           |I|I|I|I|I|I|I|I| |         |   |
 * |D|0| E |   0   |V|P|     0     |P|P|P|P|P|P|P|P|0| ExcCode | 0 | Cause
 * | | |   |       | | |           |7|6|5|4|3|2|1|0| |         |   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Cause		$13
#define R_C0_Cause		13

#define S_CauseBD		31			/* Exception in branch delay slot (R) */
#define M_CauseBD		(0x1 << S_CauseBD)
#define S_CauseCE		28			/* Coprocessor unusable (R) */
#define M_CauseCE		(0x3<< S_CauseCE)
#define S_CauseIV		23			/* Interrupt to use special interrupt vector (R/W) */
#define M_CauseIV		(0x1 << S_CauseIV)
#define S_CauseWP		22			/* Watch exception (R/W) */
#define M_CauseWP		(0x1 << S_CauseWP)
#define S_CauseIP		8			/* Interrupt cause/control */
#define M_CauseIP		(0xff << S_CauseIP)
#define S_CauseIPEXT		10			/* Interrupt causes (R) */
#define M_CauseIPEXT		(0x3f << S_CauseIPEXT)
#define S_CauseIP7		15			/* Hardware interrupt 5 or timer interrupt */
#define M_CauseIP7		(0x1 << S_CauseIP7)
#define S_CauseIP6		14			/* Hardware interrupt 4 */
#define M_CauseIP6		(0x1 << S_CauseIP6)
#define S_CauseIP5		13			/* Hardware interrupt 3 */
#define M_CauseIP5		(0x1 << S_CauseIP5)
#define S_CauseIP4		12			/* Hardware interrupt 2 */
#define M_CauseIP4		(0x1 << S_CauseIP4)
#define S_CauseIP3		11			/* Hardware interrupt 1 */
#define M_CauseIP3		(0x1 << S_CauseIP3)
#define S_CauseIP2		10			/* Hardware interrupt 0 */
#define M_CauseIP2		(0x1 << S_CauseIP2)
#define S_CauseIP1		9			/* Request software interrupt 1 (R/W) */
#define M_CauseIP1		(0x1 << S_CauseIP1)
#define S_CauseIP0		8			/* Request software interrupt 0 (R/W) */
#define M_CauseIP0		(0x1 << S_CauseIP0)
#define S_CauseExcCode		2			/* Exception code (R) */
#define M_CauseExcCode		(0x1f << S_CauseExcCode)

#define M_Cause0Fields		0x4f3f0083
#define M_CauseRFields		0xb000fc7c

/*
 * Values in the CE field
 */
#define K_CauseCE0		0			/* Coprocessor 0 in the CE field */
#define K_CauseCE1		1			/* Coprocessor 1 in the CE field */
#define K_CauseCE2		2			/* Coprocessor 2 in the CE field */
#define K_CauseCE3		3			/* Coprocessor 3 in the CE field */

/*
 * Values in the ExcCode field
 */
#define	EX_INT			0			/* Interrupt */
#define	EXC_INT			(EX_INT << S_CauseExcCode)
#define	EX_MOD			1			/* TLB modified */
#define	EXC_MOD			(EX_MOD << S_CauseExcCode)
#define	EX_TLBL		        2			/* TLB exception (load or ifetch) */
#define	EXC_TLBL		(EX_TLBL << S_CauseExcCode)
#define	EX_TLBS		        3			/* TLB exception (store) */
#define	EXC_TLBS		(EX_TLBS << S_CauseExcCode)
#define	EX_ADEL		        4			/* Address error (load or ifetch) */
#define	EXC_ADEL		(EX_ADEL << S_CauseExcCode)
#define	EX_ADES		        5			/* Address error (store) */
#define	EXC_ADES		(EX_ADES << S_CauseExcCode)
#define	EX_IBE			6			/* Instruction Bus Error (ifetch) */
#define	EXC_IBE			(EX_IBE << S_CauseExcCode)
#define	EX_DBE			7			/* Data Bus Error (data reference: load or store) */
#define	EXC_DBE			(EX_DBE << S_CauseExcCode)
#define	EX_SYS			8			/* Syscall */
#define	EXC_SYS			(EX_SYS << S_CauseExcCode)
#define	EX_SYSCALL		EX_SYS
#define	EXC_SYSCALL		EXC_SYS
#define	EX_BP			9			/* Breakpoint */
#define	EXC_BP			(EX_BP << S_CauseExcCode)
#define	EX_BREAK		EX_BP
#define	EXC_BREAK		EXC_BP
#define	EX_RI			10			/* Reserved instruction */
#define	EXC_RI			(EX_RI << S_CauseExcCode)
#define	EX_CPU			11			/* CoProcessor Unusable */
#define	EXC_CPU			(EX_CPU << S_CauseExcCode)
#define	EX_OV			12			/* Integer OVerflow */
#define	EXC_OV			(EX_OV << S_CauseExcCode)
#define	EX_TR		    	13			/* Trap instruction */
#define	EXC_TR			(EX_TR << S_CauseExcCode)
#define	EX_TRAP			EX_TR
#define	EXC_TRAP		EXC_TR
#define EX_WATCH		23			/* Watch exception */
#define EXC_WATCH		(EX_WATCH << S_CauseExcCode)
#define	EX_MCHECK	        24			/* Machine check exception */
#define	EXC_MCHECK 		(EX_MCHECK << S_CauseExcCode)


/*
 ************************************************************************
 *                   E P C   R E G I S T E R   ( 1 4 )                  *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Exception PC                            | EPC
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_EPC			$14			/* (R/W) */
#define R_C0_EPC		14

#define M_EPC0Fields		0x00000000
#define M_EPCRFields		0x00000000


/*
 ************************************************************************
 *                  P R I D   R E G I S T E R   ( 1 5 )                 *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |       r       |   Company ID  |  Procesor ID  |   Revision    | PRId
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_PRId			$15
#define R_C0_PRId		15

#define S_PRIdCoID		16			/* Company ID (R) */
#define M_PRIdCoID		(0xff << S_PRIdCoID)
#define S_PRIdImp		8			/* Implementation ID (R) */
#define M_PRIdImp		(0xff << S_PRIdImp)
#define S_PRIdRev		0			/* Revision (R) */
#define M_PRIdRev		(0xff << S_PRIdRev)

#define M_PRId0Fields		0x00000000
#define M_PRIdRFields		0xffffffff

/*
 * Values in the Company ID field
 */
#define K_PRIdCoID_MIPS		1
#define K_PRIdCoID_Broadcom	2
#define K_PRIdCoID_Alchemy	3
#define K_PRIdCoID_SiByte	4
#define K_PRIdCoID_SandCraft	5
#define K_PRIdCoID_Philips	6
#define K_PRIdCoID_NextAvailable 7 /* Next available encoding */

/*
 * Values in the implementation number field
 */
#define K_PRIdImp_Jade		0x80	/* 4Kc */
#define K_PRIdImp_Opal		0x81
#define K_PRIdImp_Ruby		0x82
#define K_PRIdImp_JadeLite	0x83	/* 4Kp, 4Km */
#define K_PRIdImp_4KEc          0x84    /* Emerald with TLB MMU */
#define K_PRIdImp_4KEmp         0x85    /* Emerald with FM MMU */
#define K_PRIdImp_4KSc          0x86    /* Coral */

#define K_PRIdImp_R3000		0x01
#define K_PRIdImp_R4000		0x04
#define K_PRIdImp_R10000	0x09
#define K_PRIdImp_R4300		0x0b
#define K_PRIdImp_R5000		0x23
#define K_PRIdImp_R5200		0x28
#define K_PRIdImp_R5400		0x54


/*
 ************************************************************************
 *               C O N F I G   R E G I S T E R   ( 1 6 )                *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | |  K  |  K  |       |M| | M |B|B| A |  A  |  M  |       |  K  |
 * |M|  2  |  U  |   r   |D|r| M |M|E| T |  R  |  T  |  0    |  0  | Config
 * | |  3  |     |       |U| |   | | |   |     |     |       |     |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Config		$16
#define R_C0_Config		16

#define S_ConfigMore		31			/* Additional config registers present (R) */
#define M_ConfigMore		(0x1 << S_ConfigMore)
#define S_ConfigMDU		20			/* Type of Multiply-Divide Unit (R) */
#define M_ConfigMDU		(0x1 << S_ConfigMDU)
#define S_ConfigMM		17			/* Merge mode for write buffer (R) */
#define M_ConfigMM		(0x3 << S_ConfigMM)
#define S_ConfigBM		16			/* Denotes subBlock (vs. sequential) burst (R) */
#define M_ConfigBM		(0x1 << S_ConfigBM)
#define S_ConfigBE		15			/* Denotes big-endian operation (R) */
#define M_ConfigBE		(0x1 << S_ConfigBE)
#define S_ConfigAT		13			/* Architecture type (R) */
#define M_ConfigAT		(0x3 << S_ConfigAT)
#define S_ConfigAR		10			/* Architecture revision (R) */
#define M_ConfigAR		(0x7 << S_ConfigAR)
#define S_ConfigMT		7			/* MMU Type (R) */
#define M_ConfigMT		(0x7 << S_ConfigMT)
#define S_ConfigK0		0			/* Kseg0 coherency algorithm (R/W) */
#define M_ConfigK0		(0x7 << S_ConfigK0)

/*
 * The following definitions are technically part of the "reserved for
 * implementations" field, but are the semi-standard definition used in
 * fixed-mapping MMUs to control the cacheability of kuseg and kseg2/3
 * references.  For that reason, they are included here, but may be
 * overridden by true implementation-specific definitions
 * For 4Kp and 4Km, these fields are valid.  For 4Kc, they are reserved.
 */
#define S_ConfigK23		28			/* Kseg2/3 coherency algorithm (FM MMU only) (R/W) */
#define M_ConfigK23		(0x7 << S_ConfigK23)
#define S_ConfigKU		25			/* Kuseg coherency algorithm (FM MMU only) (R/W) */
#define M_ConfigKU		(0x7 << S_ConfigKU)

#define M_Config0Fields		0x00000078
#define M_ConfigRFields		0x01e80000		/* For 4Kc: 0x7fe80000 */

/*
 * Cache attribute values in the K23, KU, and K0 fields of Config
 * are as given for C field fo EntryLo0 above
 */

/*
 * Values in the MDU field
 */
#define K_ConfigMDU_Fast	0			/* Fast Multiplier Array (4Kc, 4Km) */
#define K_ConfigMDU_Iterative	0			/* Iterative Multiplier (4Kp) */

/*
 * Values in the AT field
 */
#define K_ConfigAT_MIPS32	0			/* MIPS32 */
#define K_ConfigAT_MIPS64S	1			/* MIPS64 with 32-bit addresses */
#define K_ConfigAT_MIPS64	2			/* MIPS64 with 32/64-bit addresses */

/*
 * Values in the MT field
 */
// For 4K cores:  only MT = 1, 3 are valid.
#define K_ConfigMT_NoMMU	0			/* No MMU */
#define K_ConfigMT_TLBMMU	1			/* Standard TLB MMU - 4Kc */
#define K_ConfigMT_BATMMU	2			/* Standard BAT MMU */
#define K_ConfigMT_FMMMU	3			/* Standard Fixed Mapping MMU - 4Kp, 4Km */


/*
 ************************************************************************
 *         C O N F I G 1   R E G I S T E R   ( 1 6, SELECT 1 )          *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0|  MMU Size |  IS |  IL |  IA |  DS |  DL |  DA | 0 |P|W|C|E|F| Config1
 * | |           |     |     |     |     |     |     |   |C|R|A|P|P|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Config1		$16,1
#define R_C0_Config1		16

#define S_Config1MMUSize 	25			/* Number of MMU entries - 1 (R) */
#define M_Config1MMUSize 	(0x3f << S_Config1MMUSize)
#define S_Config1IS		22			/* Icache sets per way (R) */
#define M_Config1IS		(0x7 << S_Config1IS)
#define S_Config1IL		19			/* Icache line size (R) */
#define M_Config1IL		(0x7 << S_Config1IL)
#define S_Config1IA		16			/* Icache associativity - 1 (R) */
#define M_Config1IA		(0x7 << S_Config1IA)
#define S_Config1DS		13			/* Dcache sets per way (R) */
#define M_Config1DS		(0x7 << S_Config1DS)
#define S_Config1DL		10			/* Dcache line size (R) */
#define M_Config1DL		(0x7 << S_Config1DL)
#define S_Config1DA		7			/* Dcache associativity (R) */
#define M_Config1DA		(0x7 << S_Config1DA)
#define S_Config1PC		4			/* Denotes performance counters present (R) */
#define M_Config1PC		(0x1 << S_Config1PC)
#define S_Config1WR		3			/* Denotes watch registers present (R) */
#define M_Config1WR		(0x1 << S_Config1WR)
#define S_Config1CA		2			/* Denotes MIPS-16 present (R) */
#define M_Config1CA		(0x1 << S_Config1CA)
#define S_Config1EP		1			/* Denotes EJTAG present (R) */
#define M_Config1EP		(0x1 << S_Config1EP)
#define S_Config1FP		0			/* Denotes floating point present (R) */
#define M_Config1FP		(0x1 << S_Config1FP)

#define M_Config10Fields	0x80000060
#define M_Config1RFields	0x7fffff9f

/*
 * The following macro generates a table that is indexed
 * by the Icache or Dcache sets field in Config1 and
 * contains the decoded value of sets per way
 */
// CHECKME:  How is HALF() defined?  (LYT 12-20-00)
/*  Don't use these so commenting them out.  (LYT 1-15-01)
#define Config1CacheSets()	\
	HALF(64);		\
	HALF(128);		\
	HALF(256);		\
	HALF(512);		\
	HALF(1024);		\
	HALF(2048);		\
	HALF(4096);		\
	HALF(8192);
*/

/*
 * The following macro generates a table that is indexed
 * by the Icache or Dcache line size field in Config1 and
 * contains the decoded value of the cache line size, in bytes
 */
/*  Don't use these so commenting them out.  (LYT 1-15-01)
#define Config1CacheLineSize()	\
	HALF(0);		\
	HALF(4);		\
	HALF(8);		\
	HALF(16);		\
	HALF(32);		\
	HALF(64);		\
	HALF(128);		\
	HALF(256);
*/


/*
 ************************************************************************
 *                L L A D D R   R E G I S T E R   ( 1 7 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   0   |             LL Physical Address                       | LLAddr
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_LLAddr		$17
#define R_C0_LLAddr		17

#define S_LLAddrPAddr		0			/* Phys addr read by last Load Linked instruction (R) */
#define M_LLAddrPAddr		(0xfffffff << S_LLAddrPAddr)

#define M_LLAddr0Fields		0xf0000000
#define M_LLAddrRFields		0x0fffffff


/*
 ************************************************************************
 *               W A T C H L O   R E G I S T E R   ( 1 8 )              *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Watch Virtual Address                 |I|R|W| WatchLo
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_WatchLo		$18
#define R_C0_WatchLo		18

#define S_WatchLoVAddr		3			/* Watch virtual address (R/W) */
#define M_WatchLoVAddr		(0x1fffffff << S_WatchLoVAddr)
#define S_WatchLoI		2			/* Enable Istream watch (R/W) */
#define M_WatchLoI		(0x1 << S_WatchLoI)
#define S_WatchLoR		1			/* Enable data read watch (R/W) */
#define M_WatchLoR		(0x1 << S_WatchLoR)
#define S_WatchLoW		0			/* Enable data write watch (R/W) */
#define M_WatchLoW		(0x1 << S_WatchLoW)

#define M_WatchLo0Fields	0x00000000
#define M_WatchLoRFields	0x00000000

#define M_WatchLoEnables	(M_WatchLoI | M_WatchLoR | M_WatchLoW)


/*
 ************************************************************************
 *               W A T C H H I   R E G I S T E R   ( 1 9 )              *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |0|G|     0     |      ASID     |   0   |       Mask      |  0  | WatchHi
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_WatchHi		$19
#define R_C0_WatchHi		19

#define S_WatchHiG		30			/* Enable ASID-independent Watch match (R/W) */
#define M_WatchHiG		(0x1 << S_WatchHiG)
#define S_WatchHiASID		16			/* ASID value to match (R/W) */
#define M_WatchHiASID		(0xff << S_WatchHiASID)
#define S_WatchHiMask		3			/* Address inhibit mask (R/W) */
#define M_WatchHiMask		(0x1ff << S_WatchHiMask)

#define M_WatchHi0Fields	0xbf00f007
#define M_WatchHiRFields	0x00000000


/*
 ************************************************************************
 *                 D E B U G   R E G I S T E R   ( 2 3 )                *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |D|D| |L|D|H|C|I|   |D|I|   |     |         | |S|   |D|D|D|D|D|D|
 * |B|M| |S|o|a|o|B|   |B|E|   |EJTAG|DExcCode | |S|   |I|I|D|D|B|S|
 * |D| | |N|z|l|u|u|   |u|X|   | ver |         | |t|   |N|B|B|B|p|S|
 * | | |r|M|e|t|n|s| r |s|I| r |     |         |r| | r |T| |S|L| | | Debug
 * | | | | | | |t|E|   |E| |   |     |         | | |   | | | | | | |
 * | | | | | | |D|P|   |P| |   |     |         | | |   | | | | | | |
 * | | | | | | |M| |   | | |   |     |         | | |   | | | | | | |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_Debug		$23	/* EJTAG */
#define R_C0_Debug		23

#define S_DebugDBD		31			/* Debug branch delay (R) */
#define M_DebugDBD		(0x1 << S_DebugDBD)
#define S_DebugDM		30			/* Debug mode (R) */
#define M_DebugDM		(0x1 << S_DebugDM)
#define S_DebugLSNM		28			/* Load/Store Normal Memory (R/W) */
#define M_DebugLSNM		(0x1 << S_DebugLSNM)
#define S_DebugDoze		27			/* Doze (R) */
#define M_DebugDoze		(0x1 << S_DebugDoze)
#define S_DebugHalt		26			/* Halt (R) */
#define M_DebugHalt		(0x1 << S_DebugHalt)
#define S_DebugCountDM		25			/* Count register behavior in debug mode (R/W) */
#define M_DebugCountDM		(0x1 << S_DebugCountDM)
#define S_DebugIBusEP		24			/* Imprecise Instn Bus Error Pending (R/W) */
#define M_DebugIBusEP		(0x1 << S_DebugIBusEP)
#define S_DebugDBusEP		21			/* Imprecise Data Bus Error Pending (R/W) */
#define M_DebugDBusEP		(0x1 << S_DebugDBusEP)
#define S_DebugIEXI		20			/* Imprecise Exception Inhibit (R/W) */
#define M_DebugIEXI		(0x1 << S_DebugIEXI)
#define S_DebugEJTAGver		15			/* EJTAG version number (R) */
#define M_DebugEJTAGver		(0x7 << S_DebugEJTAGver)
#define S_DebugDExcCode		10			/* Debug exception code (R) */
#define M_DebugDExcCode		(0x1f << S_DebugDExcCode)
#define S_DebugSSt		8			/* Single step enable (R/W) */
#define M_DebugSSt		(0x1 << S_DebugSSt)
#define S_DebugDINT		5			/* Debug interrupt (R) */
#define M_DebugDINT		(0x1 << S_DebugDINT)
#define S_DebugDIB		4			/* Debug instruction break (R) */
#define M_DebugDIB		(0x1 << S_DebugDIB)
#define S_DebugDDBS		3			/* Debug data break store (R) */
#define M_DebugDDBS		(0x1 << S_DebugDDBS)
#define S_DebugDDBL		2			/* Debug data break load (R) */
#define M_DebugDDBL		(0x1 << S_DebugDDBL)
#define S_DebugDBp		1			/* Debug breakpoint (R) */
#define M_DebugDBp		(0x1 << S_DebugDBp)
#define S_DebugDSS		0			/* Debug single step (R) */
#define M_DebugDSS		(0x1 << S_DebugDSS)

#define M_Debug0Fields	0x20cc02c0
#define M_DebugRFields	0xce03fc3f


/*
 ************************************************************************
 *                 D E P C   R E G I S T E R   ( 2 4 )                  *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                 EJTAG Debug Exception PC                      | DEPC
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */


#define C0_DEPC			$24			/* (R/W) */
#define R_C0_DEPC		24

#define M_DEEPC0Fields		0x00000000
#define M_DEEPCRFields		0x00000000


/*
 ************************************************************************
 *                T A G L O   R E G I S T E R   ( 2 8 )                 *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                                           |   |       | | |L| |
 * |                    PA                     | r | Valid |r|L|R|r| TagLo
 * |                                           |   |       | | |F| |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_TagLo		$28
#define R_C0_TagLo		28

#define S_TagLoPA		10			/* Physical Address of cache line (R/W) */
#define M_TagLoPA		(0x3fffff << S_TagLoPA)
#define S_TagLoValid		4			/* Denotes word is valid in cache (R/W) */
#define M_TagLoValid		(0xf << S_TagLoValid)
#define S_TagLoL		2			/* Lock bit for cache tag (R/W) */
#define M_TagLoL		(0x1 << S_TagLoL)
#define S_TagLoLRF		1			/* Bit is inverted for each new cache line fill (R/W) */
#define M_TagLoLRF		(0x1 << S_TagLoLRF)

#define M_TagLo0Fields		0x00000309
#define M_TagLoRFields		0x00000000


/*
 ************************************************************************
 *        D A T A L O   R E G I S T E R   ( 2 8, SELECT 1 )             *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                           DataLo                              | DataLo
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_DataLo		$28,1			/* (R) */
#define R_C0_DataLo		28

#define M_DataLo0Fields		0x00000000
#define M_DataLoRFields		0xffffffff


/*
 ************************************************************************
 *            E R R O R E P C   R E G I S T E R   ( 3 0 )               *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Error PC                              | ErrorEPC
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_ErrorEPC		$30			/* (R/W) */
#define R_C0_ErrorEPC		30

#define M_ErrorEPC0Fields	0x00000000
#define M_ErrorEPCRFields	0x00000000


/*
 ************************************************************************
 *            D E S A V E   R E G I S T E R   ( 3 1 )                   *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                  EJTAG Register Save Value                    | DESAVE
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define C0_DESAVE		$31			/* (R/W) */
#define R_C0_DESAVE		31

#define M_DESAVE0Fields		0x00000000
#define M_DESAVERFields		0x00000000



/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* CPUCP0REGDEF_H */

