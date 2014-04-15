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
 *  File Name: cpu.h
 *     Author: Linda Yang
 *
 ******************************************************************************
 *
 * Revision History:
 *
 *      Date    Name  Comments
 *    --------  ---   ------------------------------------
 *    12/22/00  LYT   Created.
 *
 *****************************************************************************/


/* FILE_DESC ******************************************************************
//
// Purpose:
//    This file defines all the MIPS 4K related macros.
//
// Sp. Notes:
//
******************************************************************************/

#ifndef CPU_H
#define CPU_H

/*=====================*
 *  Include Files      *
 *=====================*/
#include "mem_map.h"


/*=====================*
 *  Defines            *
 *=====================*/


//-----------------------------------------------------------------------------
//      Virtual memory segments
//-----------------------------------------------------------------------------
#define KSEG_MSK                  0xE0000000
#define KSEG0BASE                 0x80000000
#define KSEG1BASE                 0xA0000000
#define KSEG2BASE                 0xC0000000
#define KSEG3BASE                 0xE0000000
#define KUSEGBASE                 0x00000000

#define KSEG0(addr)               (((addr) & ~KSEG_MSK) | KSEG0BASE)
#define KSEG1(addr)               (((addr) & ~KSEG_MSK) | KSEG1BASE)
#define KSEG2(addr)               (((addr) & ~KSEG_MSK) | KSEG2BASE)
#define KSEG3(addr)               (((addr) & ~KSEG_MSK) | KSEG3BASE)
#define KUSEG(addr)               ( (addr) & ~KSEG_MSK)

#define UNCACHED(addr)            KSEG1(addr)

#ifdef USE_CACHE
  #define CACHED(addr)            KSEG0(addr)
#else
  #define CACHED(addr)            KSEG1(addr)
#endif

#define PHYS(addr)                KUSEG(addr)

// In case the cpu performs address translation, this macro performs 
// the same translation on the given address.
#define CPU_ADDR(addr)            PHYS(addr)


//-----------------------------------------------------------------------------
//      Cache definitions
//-----------------------------------------------------------------------------
#define CACHE_OP(code, type)                    (((code) << 2) | (type))

// type: 0 = Instruction
//       1 = Data
//       2,3 = Not supported on 4K cores
// code: 0 = Index invalidate
//       1 = Index load tag
//       2 = Index store tag
//       3 = Reserved (treated as NOP)
//       4 = Hit invalidate
//       5 = Instruction fill / data hit invalidate
//       6 = Hit writeback (treated as NOP)
//       7 = Fetch and lock

#define ICACHE_INDEX_INVALIDATE                 CACHE_OP(0x0, 0)
#define ICACHE_INDEX_LOAD_TAG                   CACHE_OP(0x1, 0)
#define ICACHE_INDEX_STORE_TAG                  CACHE_OP(0x2, 0)

#define DCACHE_INDEX_WRITEBACK_INVALIDATE       CACHE_OP(0x0, 1)
#define DCACHE_INDEX_LOAD_TAG                   CACHE_OP(0x1, 1)
#define DCACHE_INDEX_STORE_TAG                  CACHE_OP(0x2, 1)

#define ICACHE_ADDR_HIT_INVALIDATE              CACHE_OP(0x4, 0)
#define ICACHE_ADDR_FILL                        CACHE_OP(0x5, 0)
#define ICACHE_ADDR_FETCH_LOCK                  CACHE_OP(0x7, 0)

#define DCACHE_ADDR_HIT_INVALIDATE              CACHE_OP(0x4, 1)
#define DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE    CACHE_OP(0x5, 1)
#define DCACHE_ADDR_HIT_WRITEBACK               CACHE_OP(0x6, 1)
#define DCACHE_ADDR_FETCH_LOCK                  CACHE_OP(0x7, 1)


//-----------------------------------------------------------------------------
//      Timer definitions
//-----------------------------------------------------------------------------
// Latency in executing the code to disable a timer.
// The latency for code in IRAM is less, since IRAM has single-cycle access.
// The value is a count of timer ticks, which would depend on the sysclk
// divider value.
#define DISABLE_TMR_DELAY_IRAM   (0x30)
#define DISABLE_TMR_DELAY        (0x80)


//-----------------------------------------------------------------------------
//      Interrupt definitions
//-----------------------------------------------------------------------------
#define INTC_NUM_IRQS        2			// firmware programmable ints

#define IRQ0_SETUP_MASK      M_StatusIM2
#define IRQ1_SETUP_MASK      M_StatusIM3

#define IRQ0_DETECT_MASK     M_CauseIP2
#define IRQ1_DETECT_MASK     M_CauseIP3


//-----------------------------------------------------------------------------
//      MIPS Processor Exception vector locations
//-----------------------------------------------------------------------------
#define VECTORS_BASE         (0x80000000)	// kseg0, 0x0
#define BEV1_VECTORS_BASE    (0xBFC00000)      	// kseg1, 0x1FC00000

#define TLBMISS_VEC_OFFSET   (0x000)
#define EXCEPT_VEC_OFFSET    (0x180)
#define INT_VEC_OFFSET       (0x200)

#define BEV1_RESET_VEC_OFFSET     (0x000)
#define BEV1_TLBMISS_VEC_OFFSET   (0x200)
#define BEV1_EXCEPT_VEC_OFFSET    (0x380)
#define BEV1_INT_VEC_OFFSET       (0x400)
#define BEV1_DEBUG_VEC_OFFSET     (0x480)


#define UNDEF_INST_VEC_LOC   (0x0)
#define PREF_ABORT_VEC_LOC   EXCEPT_VEC_LOC

#define TLBMISS_VEC_LOC      (VECTORS_BASE | TLBMISS_VEC_OFFSET)
#define EXCEPT_VEC_LOC       (VECTORS_BASE | EXCEPT_VEC_OFFSET)
#define INT_VEC_LOC          (VECTORS_BASE | INT_VEC_OFFSET)

#define RESET_VEC_LOC        (BEV1_VECTORS_BASE | BEV1_RESET_VEC_OFFSET)
#define BEV1_TBLMISS_VEC_LOC (BEV1_VECTORS_BASE | BEV1_TLBMISS_VEC_OFFSET)
#define BEV1_EXCEPT_VEC_LOC  (BEV1_VECTORS_BASE | BEV1_EXCEPT_VEC_OFFSET)
#define BEV1_INT_VEC_LOC     (BEV1_VECTORS_BASE | BEV1_INT_VEC_OFFSET)
#define BEV1_DEBUG_VEC_LOC   (BEV1_VECTORS_BASE | BEV1_DEBUG_VEC_OFFSET)


//-----------------------------------------------------------------------------
//      Stack definitions
//-----------------------------------------------------------------------------
// Main stack pointer as well as interrupt stack pointer definitions
#define INT_STACK_SIZE	(0x100)

#ifdef STK_IN_VECTMEM
  #ifdef REMAPPED_VECTOR_MEM
    #define INT_SP_BASE         (VECTMEM_REMAPPED_END)
  #else
    #define INT_SP_BASE         (VECTMEM_BOOT_END)
  #endif
#else
  #ifdef STK_IN_SRAM
    #define INT_SP_BASE           (SRAM_END)
  #else
    #ifdef REMAPPED_IRAM
      #define INT_SP_BASE         (ISRAM_REMAPPED_END)
    #else
      #define INT_SP_BASE         (ISRAM_BOOT_END)
    #endif  // REMAPPED_IRAM
  #endif  // STK_IN_SRAM
#endif

// Start main stack pointer after interrupt stack
#define MAIN_SP_BASE            (INT_SP_BASE - INT_STACK_SIZE)


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* CPU_H */
