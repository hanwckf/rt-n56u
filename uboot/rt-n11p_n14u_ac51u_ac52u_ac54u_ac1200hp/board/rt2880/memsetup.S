/* FILE_DESC *****************************************************************
//
// Purpose:
//    This file contains macros used for memory initialization.
//
// Sp. Notes:
//
// *****************************************************************************/

/*=====================*
 *  Include Files      *
 *=====================*/
#include <config.h>
#include <version.h>
#include <asm/regdef.h>

#include "include/product.h"
#include "include/mem_map.h"
#include "include/mac.inc"
#include "include/chip_reg_map.h"


/*=====================*
 *  Defines            *
 *=====================*/

.set noreorder

// SDRAM Width
#ifdef USE_SDRAM
  #ifdef SDRAM32
    #define MAC_SDRAM_WIDTH                     (MAC_SDRAM_WIDTH_32)
  #else
  #ifdef SDRAM16
    #define MAC_SDRAM_WIDTH                     (MAC_SDRAM_WIDTH_16)
  #else
    #error "SDRAM width not defined in makefile"
  #endif
  #endif
#endif

// SRAM Width
#if USE_SRAM
  #if SRAM32
    #define MAC_SRAM_WIDTH                      (MAC_WIDTH_32)
#else
  #if SRAM16
    #define MAC_SRAM_WIDTH                      (MAC_WIDTH_16)
  #else
    #error "SRAM width not defined in makefile"
  #endif
  #endif
#endif



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



// Configures the SDRAM bank.  Must be done
// before attempting SDRAM reads or writes.
// Setup modedata with 2-clk cas latency, burst length = 4.
// Configure SDRAM2 bank identically.
// 
// Uses registers t5-t7.
//
#ifdef USE_SDRAM
#define CONFIG_SDRAM()							\
									\
	li      t6, MAC_SDRAM_CONFIG_REG;				\
	li      t7, MAC_SDRAM2_CONFIG_REG;				\
	j	sdram_pgsize_board;                                 \
        nop 	;                                                       \
                                                                        \
sdram_pgsize_sim:                                                       \
	li	t5, (0 << MAC_PGSIZE_SHIFT);                            \
	j	sdram_pgsize_done;                                      \
        nop 	;                                                       \
                                                                        \
sdram_pgsize_board:                                                     \
	li	t5, (1 << MAC_PGSIZE_SHIFT);                            \
	j	sdram_pgsize_done;                                      \
        nop     ;                                                       \
                                                                        \
sdram_pgsize_done:                                                      \
	or 	t5, (	(7 << MAC_REFR_SHIFT)                      |	\
			(0 << MAC_ACTIVE_SHIFT)                    |	\
			(0 << MAC_PRECHRG_SHIFT)                   |	\
			(1 << MAC_NUMROWADR_SHIFT)                 |	\
			(1 << MAC_PRECHGOPT_SHIFT)                 |	\
			(2 << MAC_PCABIT_SHIFT)                    |	\
			(MAC_BANKTYPE_SDRAM << MAC_BANKTYPE_SHIFT) |	\
			MAC_SDRAM_WIDTH);				\
	sw      t5, 0(t6);						\
	sw      t5, 0(t7);						\
	li	t6, MAC_SDRAM_MODE_REG;					\
	li	t7, MAC_SDRAM2_MODE_REG;				\
	li	t5, (	(2 << MAC_MD_BURSTLEN_SHIFT)               |	\
			(2 << MAC_MD_LATMODE_SHIFT));			\
	sw	t5, 0(t6);						\
	sw	t5, 0(t7);



// Initializes SDRAM via the memory controller.
// Must be done before attempting to use SDRAM.
// Initializes SDRAM2 as well.
//
// Uses t4-t6.
//
#define INIT_SDRAM()							\
									\
	/* Enable SDRAM Clock */					\
	li      t6, MAC_SDRAM_CNTL_REG;					\
	li      t4, MAC_CTRL_SDRAMCLK;					\
	sw      t4, 0(t6);						\
									\
	/* Tell the MAC to initialize SDRAM */				\
	add     t5, t4, MAC_CTRL_SDRAMINI;				\
	sw      t5, 0(t6);						\
									\
	/* Wait for completion of initialization */			\
init_sdram_loop:							\
	lw      t5, 0(t6);						\
	bne     t5, t4, init_sdram_loop;				\
	nop;                             /* branch delay slot */	\
									\
									\
	/* Enable SDRAM2 Clock */					\
	li      t6, MAC_SDRAM2_CNTL_REG;				\
	li      t4, MAC_CTRL_SDRAMCLK;					\
	sw      t4, 0(t6);						\
									\
	/* Tell the MAC to initialize SDRAM2 */				\
	add     t5, t4, MAC_CTRL_SDRAMINI;				\
	sw      t5, 0(t6);						\
									\
init_sdram2_loop:							\
	lw      t5, 0(t6);						\
	bne     t5, t4, init_sdram2_loop;				\
	nop;                             /* branch delay slot */        \
									\
        /* Initialize SDRAM Refresh Control register */                 \
        /* Setup refresh rate */                                        \
        li      t6, MAC_SDRAM_REFR_CNTL_REG;                            \
        li      t5, ((0x05F << MAC_REFRESH_RATE_SHIFT) |                \
                     (1 << MAC_REFRESH_PRESCALE_SHIFT));                \
        sw      t5, 0(t6);

#endif

/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/
	.globl	memsetup
	.ent	memsetup 
memsetup:
		
	CONFIG_SDRAM()
	INIT_SDRAM()

	j	ra
	nop
	
	.end	memsetup



