/*=====================*
 *  Include Files      *
 *=====================*/
#include <config.h>
#include <version.h>
#include <asm/regdef.h>

#include "include/sysc.h"
#include "include/product.h"
#include "include/mem_map.h"
#include "include/mac.inc"
#include "include/chip_reg_map.h"
#include "include/cpu.h"


/*=====================*
 *  Defines            *
 *=====================*/

.set noreorder


// Configures the SRAM bank.  Must be done
// before attempting SRAM reads or writes.
// Setup modedata with 2-clk cas latency, burst length = 4.
// 
// Uses registers t0-t1.
//
#ifdef USE_SRAM
#define CONFIG_SRAM()							\
									\
        li      t0, MAC_SRAM_CONFIG_REG;                                \
        li      t1, (   (1 << MAC_ADDR2CS_SETUP_SHIFT)  |               \
                        (1 << MAC_WADDR_SETUP_SHIFT)    |               \
                        (1 << MAC_RADDR_SETUP_SHIFT)    |               \
                        (1 << MAC_WE_SHIFT)             |               \
                        (1 << MAC_OE_SHIFT)             |               \
                        (1 << MAC_WHOLD_SHIFT)          |               \
                        (1 << MAC_RHOLD_SHIFT)          |               \
                        (2 << MAC_BANKTYPE_SHIFT)       |               \
                        MAC_SRAM_WIDTH);                                \
        sw      t1, 0(t0);

#endif



/*=====================*
 *  External Functions *
 *=====================*/
	.globl	soc_init
	.ent	soc_init 
soc_init:

	//////////////////////////////////////////
	// S T A R T   P L L   A N D / O R   D L L      
	//////////////////////////////////////////
#ifdef CODE_IN_SDRAM
	// If current code is in SDRAM, may have problems reading and executing
	// the code if the DLL clock to the SDRAM is changed on the fly.
#else
#ifdef USE_DLL_INIT
	// Must enable DLL before enabling PLL.  PLL before DLL does not work.
	// DLL_INVERT and DLL_PHASE are defined in product.h
	li	a0, SYSC_BASE
	li	a1, DLL_INVERT
	li	a2, DLL_PHASE	

    // Disable DLL
 	lw	t1,	0x4c(a0)			// DLL Cfg Reg : offset 0x4c 
 	li	t0, ~DLL_ENABLE
	and	t1, t1, t0
	sw	t1, 0x4c(a0)

	// Setup Configuration
	// cfg = ((uint32)a2 & DLL_PHASE_MASK) << DLL_PHASE_SHIFT;
    // cfg |= a1 ? DLL_INVERT_SYSCLK : 0;
 	andi a2, a2, DLL_PHASE_MASK
	sll a2, a2, DLL_PHASE_SHIFT  
	ori	t0, a2, DLL_INVERT_SYSCLK
	movn	a2, t0, a1		// if (t) d=s
	sw	a2, 0x4c(a0)
	
	// enable the DLL
	lw	t0, 0x4c(a0)
	ori	t0, t0, DLL_ENABLE	
	sw	t0, 0x4c(a0)
	
	// Wait until dll locked
1:	lw	t0, 0x50(a0)	// DLL Stat Reg: offset 0x50
	andi	t0, t0, DLL_LOCKED
	beqz	t0, 1b
	nop
#else
	// Current graphite (rev 2) requires DLL_INVERT_SYSCLK bit to be
	// cleared to access SDRAM
	// For CODE_IN_SDRAM, bit is cleared by debugger startice.cmd file
	li	t0, DLL_CONFIG_REG
	lw	t1, 0(t0)
	li	t2, ~DLL_INVERT_SYSCLK
	and	t3, t1, t2
	sw	t3, 0(t0)
  #endif
#endif

#ifdef USE_PLL_INIT
	// PLL_DIV and PLL_MULT are defined in product.h
	li	a0, SYSC_BASE
	li	a1, (PLL_DIV - 1)
  #ifdef USE_DLL_INIT
	// Adjust for fact that system clock is divided by two when DLL enabled
	li	a2, ((PLL_MULT * 2) - 1)	// branch delay slot
  #else
	li	a2, (PLL_MULT - 1)		// branch delay slot
  #endif

 	
    //Put PLL control in its POR state. (bypass, ~enable, reset)
	lw	t0, 0x40(a0)      // PLL Ctrl Reg: offset 0x40
	ori	t0, t0, PLL_BYPASS      
	sw	t0, 0x40(a0) 
	li  t0, PLL_BYPASS | PLL_RESET
	sw  t0, 0x40(a0)     	

	// Setup configuration
	andi a2, a2, PLL_FEEDBACK_NDIV_MASK 
	sll	a2, a2, PLL_FEEDBACK_NDIV_SHIFT 

	andi a1,a1, PLL_REFCLK_MDIV_MASK 
	sll	 a1, a1, PLL_REFCLK_MDIV_SHIFT 	     
	or	a1, a1, a2     
	sw	a1, 0x44(a0)	// PLL Cfg Reg: offset 0x44              

	// Enable the pll
	// ** Must be done in two steps.
    // ** Step 1:  take pll out of reset state
    // ** Step 2:  enable pll

	// Step 1:
	lw	t0, 0x40(a0)      
	li	t1, ~PLL_RESET
	and	t0, t0, t1
	sw	t0, 0x40(a0)                        

	// Step 2:
	lw	t0, 0x40(a0)      
	ori	t0, t0, PLL_ENABLE      
	sw	t0, 0x40(a0)      

	// Wait until pll locked before selecting pll as system clock
1:	lw	t0, 0x48(a0)	// PLL Stat Reg: offset 0x48
	andi	t0, t0, PLL_LOCKED  
	beqz	t0, 1b
	nop     
	           
	// Select pll clock for system clock
	lw	t0, 0x40(a0)      
	li	t1, ~PLL_BYPASS          
	and	t0, t0, t1       
	sw	t0,0x40(a0)     
#endif


		
	// Configure ROM Bank, which at power-up is initialized in its
	// slowest mode.
	// Use the current (POR) MAC_WIDTH value, since hardware should always
	// have the correct ROM MAC_WIDTH as the POR value.

	li	t0, MAC_ROM_CONFIG_REG
	lw	t2, 0(t0)
	andi	t2, (MAC_WIDTH_MASK << MAC_WIDTH_SHIFT)
#if 0 // DEBUG - ROM set for fastest access
	li	t1, (	(1 << MAC_BYTE_EN_SHIFT)  | \
                        (1 << MAC_ADDR2CS_SETUP_SHIFT)	| \
			(1 << MAC_WADDR_SETUP_SHIFT)	| \
			(1 << MAC_RADDR_SETUP_SHIFT)	| \
			(1 << MAC_WE_SHIFT)		| \
			(1 << MAC_OE_SHIFT)		| \
			(1 << MAC_WHOLD_SHIFT)		| \
			(1 << MAC_RHOLD_SHIFT)		| \
			(2 << MAC_BANKTYPE_SHIFT) )
#else  // DEBUG - ROM set for slowest access
	li	t1, (	(0x1 << MAC_BYTE_EN_SHIFT)  | \
                        (0x3 << MAC_ADDR2CS_SETUP_SHIFT)	| \
			(0x3 << MAC_WADDR_SETUP_SHIFT)	| \
			(0x3 << MAC_RADDR_SETUP_SHIFT)	| \
			(0xF << MAC_WE_SHIFT)		| \
			(0xF << MAC_OE_SHIFT)		| \
			(0x3 << MAC_WHOLD_SHIFT)		| \
			(0x3 << MAC_RHOLD_SHIFT)		| \
			(0x2 << MAC_BANKTYPE_SHIFT) )
#endif
	or	t1, t2
	sw	t1, 0(t0)
	

#ifdef USE_SRAM
	CONFIG_SRAM()
#endif

			
			
	/////////////////////////////
	// I N I T   P A L M P A K
	/////////////////////////////

#ifdef REMAPPED_VECTOR_MEM
	// Need to remap the vector memory to 0x0 if no memory there
	li	t0, CPU_CONFIG_REG
	lw	t1, 0(t0)
	ori	t1, REMAP_VECTMEM
	sw	t1, 0(t0)
	
#endif
	
#ifdef REMAPPED_SDRAM
	// -OR- remap the first 2MB of sdram to 0x0
	// NOTE: If both REMAPPED_VECTOR_MEM and REMAPPED_SDRAM are defined, 
	// software can setup for both, but hardware will give vector_mem
	// precedence and remap it to 0x0.
	li	t0, CPU_CONFIG_REG
	lw	t1, 0(t0)
	ori	t1, REMAP_SDRAM2VEC
	sw	t1, 0(t0)
	kaiker
#endif
						
	j	ra
	nop
	
	.end	soc_init
