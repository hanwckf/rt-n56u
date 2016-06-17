/*
 *  Startup Code for MIPS32 CPU-core
 *
 *  Copyright (c) 2003	Wolfgang Denk <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include "../../autoconf.h"
#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <rt_mmap.h>

#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)
#define SDRAM_CFG0_REG RALINK_MEMCTRL_BASE+ 0x0
#define SDRAM_CFG1_REG RALINK_MEMCTRL_BASE+ 0x4
#else
#define SDRAM_CFG0_REG 					RALINK_SYSCTL_BASE + 0x0300
#define SDRAM_CFG1_REG 					RALINK_SYSCTL_BASE + 0x0304
#define RALINK_DDR_CFG0					(RALINK_MEMCTRL_BASE+0x40)
#define RALINK_DDR_CFG1					(RALINK_MEMCTRL_BASE+0x44)
#define RALINK_DDR_CFG2					(RALINK_MEMCTRL_BASE+0x48)
#define RALINK_DDR_CFG3					(RALINK_MEMCTRL_BASE+0x4c)
#define RALINK_DDR_CFG4					(RALINK_MEMCTRL_BASE+0x50)
#define RALINK_DDR_CFG8					(RALINK_MEMCTRL_BASE+0x60)
#define RALINK_DDR_CFG9					(RALINK_MEMCTRL_BASE+0x64)
#define RALINK_DDR_CFG10				(RALINK_MEMCTRL_BASE+0x68)
#endif
#define SDRAM_CFG0_ALWAYS_ONE ( 1 << 31)
#define SDRAM_CFG1_SDRAM_INIT_START ( 1 << 31)
#define SDRAM_CFG1_SDRAM_INIT_DONE ( 1 << 30)
#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD) || \
defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
#define SDRAM_CFG0_MIPSREG		s7
#define SDRAM_CFG1_MIPSREG		s8

#define DDR_CFG0_MIPSREG		s7
#define DDR_CFG1_MIPSREG		s8
#endif
#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)
#define DELAY_USEC(us)				((700*(us))/3)
#elif defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD)
#define DELAY_USEC(us)              ((60*(us))/4)
#elif defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD) 
#define DELAY_USEC(us)              ((58*(us))/3)
#else
#define DELAY_USEC(us)				((500*(us))/3)
#endif

#if defined(MT7628_ASIC_BOARD)
#if defined(ON_BOARD_DDR2)
#define MT7628_LDO_1P8V	1
#elif  defined(ON_BOARD_DDR1) && defined(ON_BOARD_64M_DRAM_COMPONENT)
#define MT7628_LDO_1P8V	1
#else
#define MT7628_LDO_2P5V	1
#endif
#endif

#define CPLL_DEFAULT_CFG		0x507
#define RVECENT(f,n) \
   b f; nop
#define XVECENT(f,bev) \
   b f     ;           \
   li k0,bev

	.set noreorder

	.globl _start
	.text
_start:
	RVECENT(reset,0)	/* U-boot entry point */
	RVECENT(reset,1)	/* software reboot */
#if defined(CONFIG_INCA_IP)
	.word INFINEON_EBU_BOOTCFG /* EBU init code, fetched during booting */
	.word 0x00000000           /* phase of the flash                    */
#elif defined(CONFIG_PURPLE)
	.word INFINEON_EBU_BOOTCFG /* EBU init code, fetched during booting */
	.word INFINEON_EBU_BOOTCFG /* EBU init code, fetched during booting */
#else
	RVECENT(romReserved,2)
#endif
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17)
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)
	XVECENT(romExcHandle,0x200)	/* bfc00200: R4000 tlbmiss vector */
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)
	XVECENT(romExcHandle,0x280)	/* bfc00280: R4000 xtlbmiss vector */
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)
	XVECENT(romExcHandle,0x300)	/* bfc00300: R4000 cache vector */
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	/* bfc00380: R4000 general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)

	/* We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */
#ifdef CONFIG_PURPLE
/* 0xbfc00400 */
	.word	0xdc870000
	.word	0xfca70000
	.word	0x20840008
	.word	0x20a50008
	.word	0x20c6ffff
	.word	0x14c0fffa
	.word	0x00000000
	.word	0x03e00008
	.word	0x00000000
	.word   0x00000000
/* 0xbfc00428 */
	.word	0xdc870000
	.word	0xfca70000
	.word	0x20840008
	.word	0x20a50008
	.word	0x20c6ffff
	.word	0x14c0fffa
	.word	0x00000000
	.word	0x03e00008
	.word	0x00000000
	.word   0x00000000
#endif /* CONFIG_PURPLE */
	.align 4
reset:
#if defined (RT2883_FPGA_BOARD) || defined (RT2883_ASIC_BOARD) || \
    defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || \
    defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD) || \
    defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD) || \
    defined (RT6855_FPGA_BOARD) || defined (RT6855_ASIC_BOARD) || \
    defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD) || \
    defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD) || \
    defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
	# Initialize the register file
	# should not be required with good software practices
	or	$1,$0, $0
	or	$2,$0, $0
	or	$3,$0, $0
	or	$4,$0, $0
	or	$5,$0, $0
	or	$6,$0, $0
	or	$7,$0, $0
	or	$8,$0, $0
	or	$9,$0, $0
	or	$10,$0, $0
	or	$11,$0, $0
	or	$12,$0, $0
	or	$13,$0, $0
	or	$14,$0, $0
	or	$15,$0, $0
	or	$16,$0, $0
	or	$17,$0, $0
	or	$18,$0, $0
	or	$19,$0, $0
	or	$20,$0, $0
	or	$21,$0, $0
	or	$22,$0, $0
	or	$23,$0, $0
	or	$24,$0, $0
	or	$25,$0, $0
	or	$26,$0, $0
	or	$27,$0, $0
	or	$28,$0, $0
	or	$29,$0, $0
	or	$30,$0, $0
	or	$31,$0, $0

#if defined (MT7628_ASIC_BOARD)
#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)	
	li	t0, RALINK_SYSCTL_BASE + 0x34
	lw	t1, 0(t0)
	ori	t1, t1, 1<<10
	//	t1, 0x04000400
	sw	t1, 0(t0)
#endif	
#endif	
# Initialize Misc. Cop0 state	

	# Read status register
	mfc0	$10, $12
	# Set up Status register:
	# Disable Coprocessor Usable bits
	# Turn off Reduce Power bit
	# Turn off reverse endian
	# Turn off BEV (use normal exception vectors)
	# Clear TS, SR, NMI bits
	# Clear Interrupt masks
	# Clear User Mode
	# Clear ERL
	# Set EXL
	# Clear Interrupt Enable
	# modify by Bruce
	#li	$11, 0x0000ff02
	li	$11, 0x00000004
	mtc0	$11, $12

	# Disable watch exceptions
	mtc0	$0, $18

	# Clear Watch Status bits
	li	$11, 0x3
	mtc0	$11, $19

	# Clear WP bit to avoid watch exception upon user code entry
	# Clear IV bit - Interrupts go to general exception vector
	# Clear software interrupts
	mtc0	$0, $13

	# Set KSeg0 to cacheable
	# Config.K0
	mfc0	$10, $16
	li	$11, 0x7
	not	$11
	and	$10, $11
	or	$10, 0x3
	mtc0	$10, $16

	# Clear Count register
	mtc0	$0, $9

	# Set compare to -1 to delay 1st count=compare
	# Also, clears timer interrupt
	li	$10, -1
	mtc0	$10, $11

	# Cache initialization routine
	# Long and needed on HW 
	# Can be skipped if using magic simulation cache flush

	# Determine how big the I$ is
/*
 ************************************************************************
 *         C O N F I G 1   R E G I S T E R   ( 1 6, SELECT 1 )          *
 ************************************************************************
 * 	
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |M|  MMU Size |  IS |  IL |  IA |  DS |  DL |  DA |Rsvd |W|C|E|F| Config1
 * | |           |     |     |     |     |     |     |     |R|A|P|P|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
	mfc0	$10, $16, 1		# .word 0x400a8001

	# Isolate I$ Line Size
	sll	$11, $10, 10
	srl	$11, 29

	# Skip ahead if No I$
	beq	$11, $0, 10f
	nop

	li	$14, 2
	sllv	$11, $14, $11		# Now have true I$ line size in bytes

	sll	$12, $10, 7
	srl	$12, 29
	li	$14, 64
	sllv	$12, $14, $12		# I$ Sets per way

	sll	$13, $10, 13
	srl	$13, 29			# I$ Assoc (-1)
	add	$13, 1
	mul	$12, $12, $13		# Total number of sets

	lui	$14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0	$0, $28
	mtc0	$0, $29

	move	$15, $12	

	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0x8, 0($14)
	add	$15, -1			# Decrement set counter

	bne	$15, $0, 1b
	add	$14, $11		# Get next line address

	# Now go through and invalidate the D$
	# Now that the I$ has been flushed, the rest of the code can be
	# moved to kseg0 and run from the cache to go faster
10:	

	
	# Isolate D$ Line Size
	sll	$11, $10, 19
	srl	$11, 29

	# Skip ahead if No D$
	beq	$11, $0, 10f
	nop

	li	$14, 2
	sllv	$11, $14, $11		# Now have true D$ line size in bytes

	sll	$12, $10, 16
	srl	$12, 29
	li	$14, 64
	sllv	$12, $14, $12		# D$ Sets per way

	sll	$13, $10, 22
	srl	$13, 29			# D$ Assoc (-1)
	add	$13, 1

	mul	$12, $12, $13		# Get total number of sets
	
	lui	$14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0	$0, $28
	mtc0	$0, $29
	mtc0	$0, $28, 2
	mtc0	$0, $29, 2

	move	$15, $12	

	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0x9, 0($14)
	add	$15, -1			# Decrement set counter

	bne	$15, $0, 1b
	add	$14, $11		# Get next line address


#if 0 //MTK: 64K I$->32K I$
      mfc0    t0, CP0_CONFIG
      or      t0,(1<<19)
      mtc0    t0, CP0_CONFIG
      nop

      mfc0    t0, CP0_CONFIG,1
      move    t1 ,t0
      and     t0,~(0x7 << 22)
      or      t0,(2 <<22)
      mtc0    t0, CP0_CONFIG,1
      nop

      mfc0    t0, CP0_CONFIG
      and     t0,~(1<<19)
      mtc0    t0, CP0_CONFIG
      nop
      nop
#endif
	#
	# Now go through and initialize the L2$
10:	

	# Check L2 cache size
/*
 ************************************************************************
 *         C O N F I G 2   R E G I S T E R   ( 1 6, SELECT 2 )          *
 ************************************************************************
 *
 *  3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |M| TU  |  TS   |  TL   |  TA   |  SU   |  SS   |  SL   |  SA   | Config2
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

	mfc0	$10, $16, 2

	# Isolate L2$ Line Size
	sll	$11, $10, 24
	srl	$11, 28

	# Skip ahead if No L2$
	beq	$11, $0, 10f
	nop

	li	$14, 2
	sllv	$11, $14, $11		# Now have true L2$ line size in bytes

	# Isolate L2$ Sets per Way
	sll	$12, $10, 20
	srl	$12, 28
	li	$14, 64
	sllv	$12, $14, $12		# D$ Sets per way

	# Isolate L2$ Associativity
	sll	$13, $10, 28
	srl	$13, 28			# D$ Assoc (-1)
	add	$13, 1

	mul	$12, $12, $13		# Get total number of sets
	
	lui	$14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear L23TagLo/L23TagHi registers
	mtc0	$0, $28, 4
	mtc0	$0, $29, 4

	move	$15, $12	

	# L2$ Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0xB, 0($14)
	add	$15, -1			# Decrement set counter

	bne	$15, $0, 1b
	add	$14, $11		# Get next line address

   
10:
	# Determine if we have a TLB 
	mfc0	$11, $16

	sll	$11, 22
	srl	$11, 29

	li	$15, 0x1	# MT = 1  => TLB
	
	bne	$11, $15, 15f
	nop

	mfc0	$10, $16, 1			# .word 0x400a8001

	sll	$11, $10, 1
	srl	$11, 26		# Number of TLB entries (-1)

	mtc0	$0, $2		# EntryLo0 
	mtc0	$0, $3		# EntryLo1
	mtc0	$0, $5		# PageMask
	mtc0	$0, $6		# Wired
	
	li	$12, 0x80000000

1:	
	mtc0	$11, $0		# Index register
	mtc0	$12, $10	# EntryHi
	ssnop			#.word 0x00000040	
	ssnop			#.word 0x00000040	
	TLBWI
	add	$12, (2<<13)	# Add 8K to the address to avoid TLB conflict with previous entry

	bne	$11, $0, 1b
	add	$11, -1
	 

15:

#endif	
#if defined(RT3350_ASIC_BOARD) 
	// force SDRAM_MD_DRV and SDRAM_MA_DRV from 8mA --> 4mA
        li      t0, RALINK_SYSCTL_BASE + 0x10
        lw      t1, 0(t0)
        nop
        or      t1, t1, (3 << 4)
        sw      t1, 0(t0)
        nop
#endif
#if defined(RT6855A_ASIC_BOARD) || defined(RT6855A_FPGA_BOARD)
	la              t0, RALINK_SYSCTL_BASE + 0x8C
	lw              t1, 0(t0)
	nop
	srl             t2, t1, 24+1
	andi            t2, t2, 0x1
	bnez            t2, 1f
	nop
	bal		rt6855A_cpu_pll	
	nop
1:

	la			t0, RALINK_SYSCTL_BASE+0x834
	lw			t1, 0(t0)
	ori			t1, t1, 1<<8
	sw			t1, 0(t0)
	nop
	la			t0, RALINK_SYSCTL_BASE+0x834
	lw			t1, 0(t0)
	li			t2, ~(1<<8)
	and			t1, t1, t2
	sw			t1, 0(t0)
	nop
#endif

#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)
#if defined(MT7620_ASIC_BOARD) || defined(MT7620_FPGA_BOARD)
	/* warm reset will skip CPU PLL CONFIG */
	la      t0, RALINK_SYSCTL_BASE+0x38
	lw      t1, 0(t0)
	srl     t1, t1, 1
	andi    t1, t1, 0x3
	bnez    t1, CPLL_DONE
	nop
#if defined(CPLL_FROM_480MHZ)
	li      a0, 1<<11
#elif defined(CPLL_FROM_XTAL)
	li      a0, 1<<12
#elif defined(CPLL_FROM_CONF)
	li      a0, CPLL_MULTI_RATIO_CFG
	sll     a0, a0, 2
	ori     a0, a0, CPLL_DIV_RATIO_CFG
	sll     a0, a0, 6
	ori     a0, a0, CPLL_SSC_CFG
#endif
#if (defined(CPLL_FROM_480MHZ)||defined(CPLL_FROM_XTAL)||defined(CPLL_FROM_CONF))
	bal     init_cpu_pll
	nop
#else
	la      t0, RALINK_SYSCTL_BASE+0x10
	lw      t1, 0(t0)
	srl     t1, t1, 4
	andi    t1, t1, 0x3
	beqz    t1, CPLL_DONE
	addiu   t2, zero, 3
	beq     t1, t2, CPLL_DONE
	li      a0, CPLL_DEFAULT_CFG
	bal     init_cpu_pll
	nop
#endif
CPLL_DONE:
#endif
#if defined(MT7628_ASIC_BOARD) || defined(MT7628_FPGA_BOARD)
	/* polling CPLL is ready */
	li		t1, DELAY_USEC(1000000)
	la		t5, RALINK_SYSCTL_BASE+0x28
1:
	lw		t2, 0(t5)
	andi	t2, t2, 0x1
	bnez	t2, CPLL_READY
	subu	t1, t1, 1
	bgtz	t1, 1b
	nop	
	la      t0, RALINK_SYSCTL_BASE+0x2c
	lw      t3, 0(t0)
	ori		t3, t3, 0x1
	sw		t3, 0(t0)
	j		CPLL_DONE
	nop
CPLL_READY:
	la		t0, RALINK_SYSCTL_BASE+0x2c
	lw		t1, 0(t0)
	li		t2, ~0x0C
	and		t1, t1, t2
	ori		t1, t1, 0xC
	sw		t1, 0(t0)
#if defined(CPUCLK_FROM_BPLL)
	la      t0, RALINK_DYN_CFG0_REG
	lw      t3, 0(t0)
	li      t5, ~((0x0F<<8)|(0x0F<<0))
	and     t3, t3, t5
	li      t5, (10<<8)|(1<<0)
	or      t3, t3, t5
	sw      t3, 0(t0)
	
	la		t0, RALINK_SYSCTL_BASE+0x2c
	lw		t3, 0(t0)
	li		t4, ~0x0F
	and     t3, t3, t4
	ori		t3, t3, 0xE
	sw		t3, 0(t0)
	lw      t3, 0(t0)
	ori     t3, t3, 0x08
	sw      t3, 0(t0)
#elif defined(CPUCLK_FROM_XTAL)
	la      t0, RALINK_DYN_CFG0_REG
	lw      t3, 0(t0)
	li      t5, ~((0x0F<<8)|(0xF<<0))
	and     t3, t3, t5
	li      t5, (1<<8)|(1<<0)
	or      t3, t3, t5
	sw		t3, 0(t0)
	lw		t3, 0(t0)
	li		t0, RALINK_SYSCTL_BASE+0x2c
	lw		t3, 0(t0)
	li		t4, ~0x0F
	and     t3, t3, t4
	ori     t3, t3, 0xD
	sw		t3, 0(t0)
#else	
	la		t0, RALINK_DYN_CFG0_REG
	lw		t3, 0(t0)
	li		t5, ~((0x0F<<8)|(0x0F<<0))
	and		t3, t3, t5
	li		t5, (10<<8)|(1<<0)
	or		t3, t3, t5
	sw		t3, 0(t0)
	la		t0, RALINK_SYSCTL_BASE+0x2C	
	lw		t3, 0(t0)
	li		t4, ~0x0F	
	and     t3, t3, t4
	ori		t3, t3, 0xC
	sw		t3, 0(t0)
	lw		t3, 0(t0)
	ori		t3, t3, 0x08
	sw		t3, 0(t0)
#endif
CPLL_DONE:
#endif

	/* SDR and DDR initialization: delay 200us
	 */
	li t0, DELAY_USEC(200+40)
	li t1, 0x1
1:
	sub t0, t0, t1
	bnez t0, 1b
	nop

#if defined(ON_BOARD_DDR1)||defined(ON_BOARD_DDR2)
 
#if defined(RT6855_FPGA_BOARD)||defined(RT6855_ASIC_BOARD) || \
    defined(MT7620_FPGA_BOARD)||defined(MT7620_ASIC_BOARD)
	/* Use default SYSCFG1 setting */
#if defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD)
	li t1, RALINK_SYSCTL_BASE + 0x14
    lw t2, 0(t1)
    nop
    and t2, ~(0x0FFF<<16)
    or t2, (0x260<<16)
    sw t2, 0(t1)
    nop
#endif
#elif defined (RT6855A_FPGA_BOARD) || defined (RT6855A_ASIC_BOARD)
	/* set DRAM IO PAD for DDR2 */
#if defined(ON_BOARD_DDR2)
#if 0
	la t0, RALINK_SYSCTL_BASE + 0x4
	lw t1, 0(t0)
	li t2, 1<<23
	or t1, t1, t2
	sw t1, 0(t0)
	nop
#endif
/*ODT ON*/
#if 1 
	la t0, RALINK_SYSCTL_BASE + 0x4
	lw t1, 0(t0)
	li t1, 0x000cc0d4
	sw t1, 0(t0)
	nop
#endif
#endif /* defined(ON_BOARD_DDR2) */
#elif defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
	/* set DRAM IO PAD for MT7628IC */
	/* DDR LDO Enable  */
	li  t1, RALINK_RGCTRL_BASE+0x100
	lw  t4, 0(t1)
	li  t2, (1<<31)
	or  t4, t4, t2
	sw  t4, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x10c
	lw  t4, 0(t1)
#if defined (MT7628_LDO_1P8V)
	j		LDO_1P8V
	nop
#elif defined (MT7628_LDO_2P5V)
	j		LDO_2P5V
	nop
#else	
	li  t1, RALINK_SYSCTL_BASE+0xC
	lw  t2, 0(t1)
	srl t2, t2, 16
	andi t2, t2, 0x1
	beqz t2, LDO_1P8V				/* KN */
	li	t1, RALINK_SYSCTL_BASE+0x10
	lw	t3, 0(t1)
	andi	t3, t3, 0x1
	bnez	t3, LDO_2P5V
#endif
LDO_1P8V:
	li	t2, ~(1<<6)
	and	t4, t4, t2
	//ori	t4, t4, 0x3	/* 1P88V */
	li  t1, RALINK_RGCTRL_BASE+0x10c
	sw	t4, 0(t1)
	j	DDRLDO_SOFT_START
LDO_2P5V:
	/* suppose external DDR1 LDO 2.5V */
	li	t2, 1<<6
	or	t4, t4, t2
	li  t1, RALINK_RGCTRL_BASE+0x10c
	sw	t4, 0(t1)

DDRLDO_SOFT_START:
	li	t1, RALINK_RGCTRL_BASE+0x10c
	lw	t2, 0(t1)
	li	t3, 1<<16
	or	t2, t2, t3
	sw	t2, 0(t1)
	li	t3, DELAY_USEC(250*50)
LDO_DELAY:
	subu t3, t3, 1
	bnez t3, LDO_DELAY
	nop
	
	li  t1, RALINK_RGCTRL_BASE+0x10c
	lw  t2, 0(t1)
	li  t3, 1<<18
	or  t2, t2, t3
	sw  t2, 0(t1)

SET_RG_BUCK_FPWM:
	li	t1, RALINK_RGCTRL_BASE+0x104
	lw	t2, 0(t1)
	ori t2, t2, 1<<10
	sw	t2, 0(t1)

DDR_PAD_CFG:
	/* clean CLK PAD */
	li  t1, RALINK_RGCTRL_BASE+0x704
	lw  t2, 0(t1)
	li	t8, 0xFFFFF0F0
	and    t2, t2, t8
	/* clean CMD PAD */
	li  t1, RALINK_RGCTRL_BASE+0x70c
	lw  t3, 0(t1)
	li  t8, 0xFFFFF0F0
	and    t3, t3, t8
	/* clean DQ IPAD */
	li  t1, RALINK_RGCTRL_BASE+0x710
	lw  t4, 0(t1)
	li  t8, 0xFFFFF8FF
	and    t4, t4, t8
	/* clean DQ OPAD */
	li  t1, RALINK_RGCTRL_BASE+0x714
	lw  t5, 0(t1)
	li  t8, 0xFFFFF0F0
	and    t5, t5, t8
	/* clean DQS IPAD */
	li  t1, RALINK_RGCTRL_BASE+0x718
	lw  t6, 0(t1)
	li  t8, 0xFFFFF8FF
	and    t6, t6, t8
	/* clean DQS OPAD */
	li  t1, RALINK_RGCTRL_BASE+0x71c
	lw  t7, 0(t1)
	li  t8, 0xFFFFF0F0
	and    t7, t7, t8


	li	t1, RALINK_SYSCTL_BASE+0xC
	lw	t9, 0(t1)
	srl	t9, t9, 16
	andi t9, t9, 0x1	
	bnez t9, MT7628_AN_DDR1_PAD
MT7628_KN_PAD:
	li	t8, 0x00000303
	or	t2, t2, t8
	or	t3, t3, t8
	or	t5, t5, t8
	or	t7,	t7, t8
	li	t8, 0x00000000
	or	t4, t4, t8
	or	t6, t6, t8
	j	SET_PAD_CFG
MT7628_AN_DDR1_PAD:
	li	t1, RALINK_SYSCTL_BASE+0x10
	lw	t1, 0(t1)
	andi	t1, t1, 0x1
	beqz t1, MT7628_AN_DDR2_PAD
	li  t8, 0x00000C0C
	or	t2, t2, t8
    li  t8, 0x00000202
	or	t3, t3, t8
    li  t8, 0x00000707
	or	t5, t5, t8
	li  t8, 0x00000C0C
	or	t7, t7, t8
	li  t8, 0x00000000
	or	t4, t4, t8
	or	t6, t6, t8
	j	SET_PAD_CFG
MT7628_AN_DDR2_PAD:
	li  t8, 0x00000C0C
	or  t2, t2, t8
	li  t8, 0x00000202
	or  t3, t3, t8
	li  t8, 0x00000404
	or  t5, t5, t8
	li  t8, 0x00000C0C
	or  t7, t7, t8
	//li  t8, 0x00000200
	li	t8, 0x00000000		//ODT off
	or  t4, t4, t8
	or  t6, t6, t8

SET_PAD_CFG:
	li  t1, RALINK_RGCTRL_BASE+0x704
	sw	t2, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x70c
	sw	t3, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x710
	sw	t4, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x714
	sw	t5, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x718
	sw	t6, 0(t1)
	li  t1, RALINK_RGCTRL_BASE+0x71c
	sw	t7, 0(t1)
#else
	/* DDR initialization: reg SYSCFG1[25:16]: 
	 * ODT disabled, LVCMOS=1, half drive, turn ON RT3662 DDR IO ODT as 150 ohm when read DRAM
	 */	
	li t1, RALINK_SYSCTL_BASE + 0x14
	lw t2, 0(t1)
	nop
	and t2, ~(0x3FF<<16)
	or t2, (0x361<<16)
	sw t2, 0(t1)
	nop
#endif	

	/* DDR initialization: reset pin to 0
	 */
#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)
	li t1, RALINK_SYSCTL_BASE + 0x40
#else	
	li t1, RALINK_SYSCTL_BASE + 0x34
	lw t2, 0(t1)
	and t2, ~(0x1<<10)
#endif
	sw t2, 0(t1)
	nop

#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD) || \
	defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)

	li t0, DELAY_USEC(200+40)
	li t1, 0x1
1:
	sub t0, t0, t1
	bnez t0, 1b
	nop
#endif
	/* DDR initialization: wait til reg DDR_CFG1 bit 21 equal to 1 (ready)
	 */
DDR_READY:
	li t1, RALINK_MEMCTRL_BASE + 0x44 	//DDR_CFG1
	lw t0, 0(t1)
	nop
	and t2, t0, (1<<21)
	beqz t2, DDR_READY
	nop

	/* DDR initialization:
	 */
#if defined(RT6855_FPGA_BOARD)||defined(RT6855_ASIC_BOARD) || \
    defined(MT7620_FPGA_BOARD)||defined(MT7620_ASIC_BOARD) || \
    defined(MT7628_FPGA_BOARD)||defined(MT7628_ASIC_BOARD)
	 /*   fpga/asic: reg DDR_CFG2 -- set bit[30]=0 as DDR1 mode when DDR1
	  *   fpga/asic: reg DDR_CFG2 -- set bit[30]=1 as DDR2 mode when DDR2
	  *   fpga/asic: reg DDR_CFG2 -- set bit[6:4]=3'b011 when DDR1
	  *   fpga/asic: reg DDR_CFG2 -- set bit[6:4]=3'b100 when DDR2
	  */
	li t1, RALINK_MEMCTRL_BASE + 0x48	//DDR_CFG2
	lw t0, 0(t1)
	nop
	and t0, ~(1<<30)
#if ON_BOARD_DDR2
	and t0, ~(7<<4)
	or t0, (4<<4)
	or t0, (1<<30)
#if defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD) 
	or t0, (1<<11)
#endif	
#elif ON_BOARD_DDR1
	and t0, ~(7<<4)
	or t0, (3<<4)
#endif
#if defined(MT7628_FPGA_BOARD)
	li t0, 0x28000033
#endif
	sw t0, 0(t1)
	nop
#endif /* defined(RT6855_FPGA_BOARD)||defined(RT6855_ASIC_BOARD) */

/* RT3883 and RT6855 will share below setting, RT3352 no boot from NOR */
	/*
	 *   fpga: reg DDR_CFG3 -- disable DLL
	 *   asic: reg DDR_CFG3 -- ODT enable (bit 6,2) = 2'b10 when 6855/3883 DDR2
	 *   fpga/asic: reg DDR_CFG3 -- ODT enable (bit 6,2) = 2'b00 when 6855 DDR1
	 *   fpga/asic: reg DDR_CFG3[10][5:3] = 4'b0000 when 6855 DDR1
	 */
	li t1, RALINK_MEMCTRL_BASE + 0x4c 	////DDR_CFG3
	lw t2, 0(t1)
#ifdef ON_BOARD_DDR2
	#disable ODT; reference board ok, ev board fail
#if defined(MT7620_ASIC_BOARD) 	|| defined(MT7628_ASIC_BOARD)
	and t2, ~(1<<6)
	or	t2, (1<<2)
#else
	#enable ODT; both ok
	or t2, (1<<6)	
	and t2, ~(1<<2)
#endif
#elif ON_BOARD_DDR1
	and t2, ~(1<<10)
	and t2, ~(7<<3)
#if defined(MT7628_FPGA_BOARD)
	and t2, 0
	or t2, 0x3
#endif
#endif
#if defined(RT3883_FPGA_BOARD) || defined(RT6855_FPGA_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7628_FPGA_BOARD)
	or t2, 0x1
#endif
#if (!defined(RT6855A_ASIC_BOARD) && !defined(MT7620_ASIC_BOARD) && !defined(MT7628_ASIC_BOARD))
	sw t2, 0(t1)
	nop
#endif	

#ifdef RALINK_DDR_OPTIMIZATION
	/* DDR: set Burst Length=4 in 32 bits dram bus for better performance
	 *          Burst Length=8 in non 32 bits dram bus
	 */
	li	t0, RALINK_MEMCTRL_BASE + 0x48
	lw	t1, 0(t0)
	nop
	and	t1, 0xffffff88
	or	t1, (CAS_VALUE<<CAS_OFFSET)
	or	t1, (BL_VALUE<<BL_OFFSET)
	sw	t1, 0(t0)
        nop

	li	t0, RALINK_MEMCTRL_BASE + 0x4c
	lw	t1, 0(t0)
	nop
	and	t1, 0xffffffc7
	or	t1, (AdditiveLatency_VALUE<<AdditiveLatency_OFFSET)
	sw	t1, 0(t0)
#endif
#if defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)
	li  t0, RALINK_MEMCTRL_BASE + 0x50
	lw  t1, 0(t0)
	li	t2, ~(0x01F|0x0F0)
	and	t1, t1, t2
#if ON_BOARD_DDR2
#if defined (ON_BOARD_256M_DRAM_COMPONENT)
	ori t1, t1, 7
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	ori t1, t1, 9
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	ori t1, t1, 9
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	ori	t1, t1, 9
#endif
#endif
	sw	t1, 0(t0)
	nop
#elif defined(MT7628_FPGA_BOARD) || defined(MT7620_FPGA_BOARD)
	li  t0, RALINK_MEMCTRL_BASE + 0x50
	li  t1, 0x0
	sw      t1, 0(t0)
	nop
#endif
#if defined (RT3352_FPGA_BOARD) || defined (RT3883_FPGA_BOARD) || defined (RT6855_FPGA_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7628_FPGA_BOARD)
	/* DDR initialization: DDR_CFG0 bit 12:0 (refresh interval) to 0x64
	 * Note. this may have a bad affect on efficiency if the clock rate is 40MHz
	 */
	li t1, RALINK_MEMCTRL_BASE + 0x40
	lw t2, 0(t1)
	nop
	and t2, ~(0xfff)
#if defined(ON_BOARD_DDR1)
#if defined(MT7628_FPGA_BOARD) || defined(MT7620_FPGA_BOARD)
	li t2, 0x110860f9
#else
	li t2, 0x21086141
#endif
#else
	or t2, 0x64
#endif	
	sw t2, 0(t1)
	nop
#endif

#if 0
	/* data output (DQ) delay */
	li t1, RALINK_MEMCTRL_BASE + 0x60
	li t2, 0xffffffff
	sw t2, 0(t1)
	nop
	li t1, RALINK_MEMCTRL_BASE + 0x64
	li t2, 0xffffffff
	sw t2, 0(t1)
	nop
#endif

	/* DDR initialization: config size and width on reg DDR_CFG1
	 */
#if defined(ON_BOARD_DDR2)
#if defined (RT6855_ASIC_BOARD) || defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)
#ifdef ON_BOARD_128M_DRAM_COMPONENT
        li t6, 0x222A3323
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
        li t6, 0x222e2323
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
        li t6, 0x22322323 
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
        li t6, 0x22362323
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
		li t6, 0x223a2323		
#else
        DRAM Component not defined
#endif
#elif defined (RT6855_FPGA_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7628_FPGA_BOARD)
#ifdef ON_BOARD_128M_DRAM_COMPONENT
        li t6, 0x122A3121
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
        li t6, 0x222e2323	//0x122E3121
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
        li t6, 0x12323121
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
        li t6, 0x12363121
#endif

#elif defined (RT3352_FPGA_BOARD) || defined(RT3883_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || defined(RT3883_ASIC_BOARD)
#ifdef ON_BOARD_128M_DRAM_COMPONENT
	li t6, 0x222A3323
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t6, 0x222E3323
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t6, 0x22323323
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t6, 0x22363323
#else
	DRAM Component not defined
#endif

#else /* RT6855A setting in below lookup table*/

#endif

#elif defined(ON_BOARD_DDR1)

#if defined(RT6855_ASIC_BOARD) || defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)
#ifdef ON_BOARD_128M_DRAM_COMPONENT
        li t6, 0x332A3434
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
        li t6, 0x222E2324	//use aggressive cfg //0x332e2434
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
        li t6, 0x33322434
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
        li t6, 0x33362434
#endif
#elif defined(RT6855_FPGA_BOARD) || defined(MT7620_FPGA_BOARD) 
#ifdef ON_BOARD_128M_DRAM_COMPONENT
        li t6, 0x122A3111
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
        li t6, 0x222e2113	//0x122E3111
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
        li t6, 0x12323111
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
        li t6, 0x12363111
#endif
#elif defined(MT7628_FPGA_BOARD)
#ifdef ON_BOARD_128M_DRAM_COMPONENT
        li t6, 0x202A2121
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
        li t6, 0x202E2121
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
        li t6, 0x20322121
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
        li t6, 0x20362121
#endif
#else /* RT6855A setting in below lookup table*/

#endif

#endif /* end of setting DDR_CFG1 */

#ifdef ON_BOARD_DDR_WIDTH_16
	or t6, (1<<17)
	and t6, ~(1<<16)
#elif defined (ON_BOARD_DDR_WIDTH_8)
	and t6, ~(1<<17)
	or t6, (1<<16)
#else
	DDR width not defined
#endif
	/* CONFIG DDR_CFG1[13:12] about TOTAL WIDTH */
	and t6, ~(3<<12)
#ifdef ON_BOARD_32BIT_DRAM_BUS
	or t6, (3<<12)
#elif defined (ON_BOARD_16BIT_DRAM_BUS)
	or t6, (2<<12)
#else
	DRAM bus not defined
#endif
#if !defined(RT6855A_ASIC_BOARD) && !defined(MT7620_ASIC_BOARD)	&& !defined(MT7628_ASIC_BOARD)
	li t5, RALINK_MEMCTRL_BASE + 0x44
	sw t6, 0(t5)
	nop
	j SDRAM_INIT_DOWN
	nop
#endif

#if defined(RT6855A_ASIC_BOARD)
	/* RT6855A DDR configurations */
	/* abstract DRAM_SPEED */
	la		t0, RALINK_SYSCTL_BASE+0x8C
	lw		t6, 0(t0)
	nop

	srl		t5, t6, 26
	andi	t5, t5, 0x1
	
	srl		t8, t6, 8
	andi	t8, t8, 0x3
	addiu	t7, zero, 2
	sub		t8, t8, t7
	bgez	t8, 1f			/* Large package size*/
	nop
	addiu	t5, t5, 2		/* t5 is column index, DDR 166MHZ and 125MHZ shoud add 2
					  ftom DRAM_SPEED for RT6855 case */
1:					  
#if defined(ON_BOARD_DDR1)	
	sll		t5, t5, 2
	lui		t0, %hi(DDR1_CFG2_TBL)
	ori		t0, t0, %lo(DDR1_CFG2_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	nop
	la		t0, RALINK_DDR_CFG2
	sw		t1, 0(t0)
	nop
	lui		t0, %hi(DDR1_CFG3_TBL)
	ori		t0, t0, %lo(DDR1_CFG3_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	la		t0, RALINK_DDR_CFG3
	sw		t1, 0(t0)
	nop

	/* DQS delay use table value */	
	lui		t0, %hi(DDR1_CFG4_TBL)
	ori		t0, t0, %lo(DDR1_CFG4_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	nop
#if 0 
	/* DQS delay use DLL detected value then fixed the delay with a constant delay 8 */
	la		t0, RALINK_SYSCTL_BASE + 0x18
	lw		t2, 0(t0)
	andi	t3,	t2, 0x1F
	srl		t2, t2, 5
	addiu	t6, zero, 5
	mul		t0, t2, t6	
	addu	t0, t0, t3
	subu	t0, t0, 8
	div		t0, t6
	mflo	t2	
	mfhi	t3
	nop
	nop
	sll		t2, t2, 5
	or		t2, t2, t3
	sll		t4, t2, 9
	or		t2, t2, t4
	sll		t2, t2, 5
	li		t3, ~(((0x1FF<<9)|0x1FF)<<5)
	and		t1, t1, t3
	or		t1, t1, t2
#endif
	la		t0, RALINK_DDR_CFG4
	sw		t1, 0(t0)
	nop
#endif	
#if defined(ON_BOARD_DDR2)
	sll		t5, t5, 2
	lui		t0, %hi(DDR2_CFG2_TBL)
	ori		t0, t0, %lo(DDR2_CFG2_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	nop
	la		t0, RALINK_DDR_CFG2
	sw		t1, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG3_TBL)
	ori		t0, t0, %lo(DDR2_CFG3_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	nop
	
	la		t0, RALINK_DDR_CFG3
#if 0
	lw		t1, 0(t0)
	/*enable 100% drive strength*/
	and		t1, ~(0x1<<1)
#endif
	sw		t1, 0(t0)
	nop
	
	/* DQS delay use table value */	
	lui		t0, %hi(DDR2_CFG4_TBL)
	ori		t0, t0, %lo(DDR2_CFG4_TBL)
	addu	t0, t0, t5
	lw		t1, 0(t0)
	nop
#if 0
	/* DQS delay use DLL detected value then fixed the delay with a constant delay 8 */
	la		t0, RALINK_SYSCTL_BASE + 0x18
	lw		t2, 0(t0)
	andi	t3,	t2, 0x1F
	srl		t2, t2, 5
	addiu	t6, zero, 5
	mul		t0, t2, t6	
	addu	t0, t0, t3
	subu	t0, t0, 8
	div		t0, t6
	mflo	t2	
	mfhi	t3
	nop
	nop
	sll		t2, t2, 5
	or		t2, t2, t3
	sll		t4, t2, 9
	or		t2, t2, t4
	sll		t2, t2, 5
	li		t3, ~(((0x1FF<<9)|0x1FF)<<5)
	and		t1, t1, t3
	or		t1, t1, t2
#endif
	la		t0, RALINK_DDR_CFG4
	sw		t1, 0(t0)
	nop

#if 0
	la		t0, RALINK_DDR_CFG9
	li		t1, 0x88880000
	sw		t1, 0(t0)
	nop
#endif
#endif		
2:	

#if defined(ON_BOARD_DDR1)	
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	addiu 	t4, zero, 1
#elif ON_BOARD_128M_DRAM_COMPONENT
	addiu 	t4, zero, 2
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu 	t4, zero, 3
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	addiu 	t4, zero, 4
#else
	DRAM Component not defined
#endif
#endif
#if defined(ON_BOARD_DDR2)	
#if defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu 	t4, zero, 3
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	addiu 	t4, zero, 4
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	addiu 	t4, zero, 5
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	addiu 	t4, zero, 6
#else
#error	"DRAM Component not defined"
#endif
#endif
	srl		t3, t6, 24
	andi	t3, t3, 0x3
	subu	t3, t3, 2	
	srl		t5, t6, 26
	andi	t5, t5, 0x1
	bgez	t8, 1f
	nop
	addiu	t5, t5, 2	/* t5 is column index, DDR 166MZH and 125MHZ shoud add 2
					   ftom DRAM_SPEED for RT6855 case */
1:
#if defined(ON_BOARD_DDR1)
	addiu	t6, zero, 4
	subu	t4, t4, 1
	mul		t2, t4, t6
	nop
	addu	t2, t2, t5
	sll		t2, t2, 2
	lui		t0, %hi(DDR1_CFG0_TBL)
	ori		t0, t0, %lo(DDR1_CFG0_TBL)
	addu	t0, t0, t2
	lw		DDR_CFG0_MIPSREG, 0(t0)
	nop
	lui		t0, %hi(DDR1_CFG1_TBL)
	ori		t0, t0, %lo(DDR1_CFG1_TBL)
	addu	t0, t0, t2
	lw		DDR_CFG1_MIPSREG, 0(t0)
	nop
#endif	
#if defined(ON_BOARD_DDR2)
	addiu	t6, zero, 4
	subu	t4, t4, 3
	mul		t2, t4, t6
	nop
	addu	t2, t2, t5
	sll		t2, t2, 2
	lui		t0, %hi(DDR2_CFG0_TBL)
	ori		t0, t0, %lo(DDR2_CFG0_TBL)
	addu	t0, t0, t2
	lw		DDR_CFG0_MIPSREG, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG1_TBL)
	ori		t0, t0, %lo(DDR2_CFG1_TBL)
	addu	t0, t0, t2
	lw		DDR_CFG1_MIPSREG, 0(t0)
	nop
#endif	
2:
	la		t5, RALINK_DDR_CFG0
	sw		DDR_CFG0_MIPSREG, 0(t5)
	nop
	la		t5, RALINK_DDR_CFG1
	sw		DDR_CFG1_MIPSREG, 0(t5)
	nop
	j 		SDRAM_INIT_DOWN
	nop
#endif	/* defined(RT6855A_ASIC_BOARD) */

#if defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)

#if defined(ON_BOARD_DDR1)
#if defined (ON_BOARD_64M_DRAM_COMPONENT)
	addiu	t4, zero, 0
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	addiu	t4, zero, 1
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu 	t4, zero, 2
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	addiu 	t4, zero, 3
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	addiu 	t4, zero, 4
#else
	DRAM Component not defined
#endif
#endif /* defined(ON_BOARD_DDR1) */

#if defined(ON_BOARD_DDR2)
#if defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu 	t4, zero, 0
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	addiu 	t4, zero, 1
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	addiu 	t4, zero, 2
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	addiu 	t4, zero, 3
#else
#error	"DRAM Component not defined"
#endif
#endif	/* defined(ON_BOARD_DDR2) */

#if	defined(CPLL_FROM_480MHZ)|| defined (CPUCLK_FROM_BPLL)
	addiu	t6, zero, 1
#else
	addiu	t6, zero, 0
#endif
	
	sll		t2, t4, 1
	addu	t2, t2, t6
	sll		t2, t2, 2
#if defined(ON_BOARD_DDR1)
	lui     t0, %hi(DDR1_CFG2_TBL)
	ori     t0, t0, %lo(DDR1_CFG2_TBL)
	addu    t0, t0, t2
	lw      t7, 0(t0)
	nop
	lui     t0, %hi(DDR1_CFG3_TBL)
	ori     t0, t0, %lo(DDR1_CFG3_TBL)
	addu    t0, t0, t2
	lw      t8, 0(t0)
	nop
	lui     t0, %hi(DDR1_CFG4_TBL)
	ori     t0, t0, %lo(DDR1_CFG4_TBL)
	addu    t0, t0, t2
	lw      t9, 0(t0)
	nop
	lui     t0, %hi(DDR1_CFG0_TBL)
	ori     t0, t0, %lo(DDR1_CFG0_TBL)
	addu    t0, t0, t2
	lw      DDR_CFG0_MIPSREG, 0(t0)
	nop
	lui     t0, %hi(DDR1_CFG1_TBL)
	ori     t0, t0, %lo(DDR1_CFG1_TBL)
	addu    t0, t0, t2
	lw      DDR_CFG1_MIPSREG, 0(t0)
	nop
#endif
#if defined(ON_BOARD_DDR2)	
	lui		t0, %hi(DDR2_CFG2_TBL)
	ori		t0, t0, %lo(DDR2_CFG2_TBL)	
	addu	t0, t0, t2
	lw		t7, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG3_TBL)
	ori		t0, t0, %lo(DDR2_CFG3_TBL)
	addu	t0, t0, t2
	lw		t8, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG4_TBL)
	ori		t0, t0, %lo(DDR2_CFG4_TBL)
	addu	t0, t0, t2
	lw		t9, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG0_TBL)
	ori		t0, t0, %lo(DDR2_CFG0_TBL)
	addu	t0, t0, t2
	lw		DDR_CFG0_MIPSREG, 0(t0)
	nop
	lui		t0, %hi(DDR2_CFG1_TBL)
	ori		t0, t0, %lo(DDR2_CFG1_TBL)
	addu	t0, t0, t2	
	lw		DDR_CFG1_MIPSREG, 0(t0)
	nop
#endif
	la		t0, RALINK_DDR_CFG2
	sw		t7, 0(t0)
	nop
	la		t0, RALINK_DDR_CFG3
	sw		t8, 0(t0)
	nop
	la		t0, RALINK_DDR_CFG4
	sw		t9, 0(t0)
	nop
#if defined(ON_BOARD_DDR2)	
#if defined(MT7620_ASIC_BOARD) 	
	la		t0, RALINK_DDR_CFG10
	li		t1, 0x40404848
	sw		t1, 0(t0)
#endif
#endif

#if defined(MT7628_ASIC_BOARD)
	li  t1, RALINK_SYSCTL_BASE+0xC
	lw  t9, 0(t1)
	srl t9, t9, 16
	andi t9, t9, 0x1
	beqz t9, 1f
	la      t0, RALINK_DDR_CFG8
	li      t1, 0x00008282
	sw      t1, 0(t0)
	la      t0, RALINK_DDR_CFG9
	li      t1, 0x00008383
	sw      t1, 0(t0)
1:	
#endif
    la      t0, RALINK_DDR_CFG0
	sw		DDR_CFG0_MIPSREG, 0(t0)
	nop
	la		t0, RALINK_DDR_CFG1
	sw		DDR_CFG1_MIPSREG, 0(t0)
	nop

	j 		SDRAM_INIT_DOWN
	nop
#endif	/* defined(MT7620_ASIC_BOARD) */

#endif /* defined(ON_BOARD_DDR1)||defined(ON_BOARD_DDR2) */


#ifdef ON_BOARD_SDR
SDR_INIT:
#if defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)
	li t1, RALINK_SYSCTL_BASE + 0x40
	sw zero, 0(t1)
	nop
#endif
	/* SDR initialization: SDRAM_CFG0
	 */
	li t5, SDRAM_CFG0_REG
	lw t6, 0(t5)
	nop
	and t6, 0xF0000000
#ifdef FPGA_BOARD
#ifdef RT2880_FPGA_BOARD
#ifdef RT2880_MP
	nop
	or t6, 0x01825282
	//or t6, 0x01815282
	nop	
#else /* RT2880_SHUTTLE */
	or t6, 0x91825282
	//or t6, 0x91815282
#endif
#elif defined(RT6855_FPGA_BOARD) || defined (MT7620_FPGA_BOARD) || defined (MT7628_FPGA_BOARD)
	or t6, 0xD1825272
	//or t6, 0xD1916292
#else //2883, 3052, 3352, 3883, 5350 fpga
	nop
	or t6, 0xD1825272
	//or t6, 0x01815282
	nop	
#endif
#else //ASIC_BOARD
#if defined(RT6855_ASIC_BOARD) || defined (MT7620_ASIC_BOARD) || defined (MT7628_ASIC_BOARD)
        or t6, 0xD1916292
#else	
	or t6, 0xD1825272
#endif
#endif
	nop
#if (!defined(RT6855A_ASIC_BOARD) && !defined(MT7620_ASIC_BOARD) && !defined(MT7628_ASIC_BOARD))
	sw t6, 0(t5)
	nop
#endif

	li t5, SDRAM_CFG1_REG
#ifdef ASIC_BOARD
/*
 *	Turn on SDRAM RBC (BIT 29 in SDRAM_CFG1, offset 0x4) in RT3052.
 *	  RT2880 RBC bit is Reserved bit, and change the same value for RT2880 and RT3052
 *	  Original 0x81xx0600 -> 0xa1xx0600
 *		by bobtseng, 2008.7.7.
 */
#if defined (ON_BOARD_16M_DRAM_COMPONENT)
	li t6, 0xa0000668
#elif defined (ON_BOARD_64M_DRAM_COMPONENT)
	li t6, 0xa1010600
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t6, 0xa1110600
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t6, 0xa1120300
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t6, 0xa1220600
#elif defined(ON_BOARD_DDR1)||defined(ON_BOARD_DDR2)
#else
	DRAM Component not defined
#endif
#ifdef ON_BOARD_32BIT_DRAM_BUS
	and t6, 0xFEFFFFFF
	or t6, (1<<24)
#elif defined ON_BOARD_16BIT_DRAM_BUS
	and t6, 0xFEFFFFFF
#else
	DRAM bus not defined
#endif

#else /* not ASIC_BOARD */

#ifdef ON_BOARD_64M_DRAM_COMPONENT
	li t6, 0xa1010096
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t6, 0xa1110096
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t6, 0xa112004B
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t6, 0xa1220096
#else
	DRAM Component not defined
#endif

#ifdef ON_BOARD_32BIT_DRAM_BUS
	and t6, 0xFEFFFFFF
	or t6, (1<<24)
#elif defined (ON_BOARD_16BIT_DRAM_BUS)
	and t6, 0xFEFFFFFF
#else
	DRAM bus not defined
#endif
#endif



DO_SDRINIT:
	nop
#if (!defined(RT6855A_ASIC_BOARD) && !defined(MT7620_ASIC_BOARD) && !defined(MT7628_ASIC_BOARD))
	sw 		t6, 0(t5)
	nop
#endif

#if defined(RT6855A_ASIC_BOARD)
	move	t1, zero	/* force to 140Mhz case */
#if defined (ON_BOARD_16M_DRAM_COMPONENT)
	addiu	t4, zero, 0
#elif defined (ON_BOARD_64M_DRAM_COMPONENT)
	addiu	t4, zero, 1
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	addiu	t4, zero, 2
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu	t4, zero, 3
#else
	DRAM Component not defined
#endif	
	lui		t0, %hi(SDR_CFG0_TBL)
	ori		t0, t0, %lo(SDR_CFG0_TBL)
	sll		t3, t1, 2
	addu	t0, t0, t3
	lw		SDRAM_CFG0_MIPSREG, 0(t0)
	nop
	addiu	t3, zero, 2
	mul		t2, t4, t3
	nop
	addu	t2, t2, t1
	sll		t2, t2, 2
	lui		t0, %hi(SDR_CFG1_TBL)
	ori		t0, t0, %lo(SDR_CFG1_TBL)
	addu	t0, t0, t2
	lw		SDRAM_CFG1_MIPSREG, 0(t0)
	nop
	la		t0, SDRAM_CFG0_REG
    sw		SDRAM_CFG0_MIPSREG, 0(t0)
	nop
	la		t0, SDRAM_CFG1_REG
	sw		SDRAM_CFG1_MIPSREG, 0(t0)
	nop
	j			WAIT_SDRAM_INIT_DOWN
	nop
#endif /* defined(RT6855A_ASIC_BOARD) */

#if defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)

#if defined (ON_BOARD_64M_DRAM_COMPONENT)
	addiu	t4, zero, 0
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	addiu	t4, zero, 1
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	addiu	t4, zero, 2
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	addiu	t4, zero, 3
#else
	DRAM Component not defined
#endif

	li		t0, RALINK_SYSCTL_BASE + 0x10
	lw		t1, 0(t0)
	srl		t1, t1, 4
	andi	t1, t1, 0x3
	addiu	t2, zero, 3
	addiu	t6, zero, 1
	subu	t2, t2, t1 	
	movn	t6, zero, t2

	sll		t2, t4, 1
	addu	t2, t2, t6
	sll		t2, t2, 2
	
	lui		t0, %hi(SDR_CFG0_TBL)
	ori		t0, t0, %lo(SDR_CFG0_TBL)
	addu	t0, t0, t2
	lw		SDRAM_CFG0_MIPSREG, 0(t0)
	nop
	lui		t0, %hi(SDR_CFG1_TBL)
	ori		t0, t0, %lo(SDR_CFG1_TBL)
	addu	t0, t0, t2
	lw		SDRAM_CFG1_MIPSREG, 0(t0)
#if defined(RALINK_SDR_POWERSAVE)
#endif	
	la		t0, SDRAM_CFG0_REG
  sw		SDRAM_CFG0_MIPSREG, 0(t0)
	nop
	la		t0, SDRAM_CFG1_REG
	sw		SDRAM_CFG1_MIPSREG, 0(t0)
	nop
	j			WAIT_SDRAM_INIT_DOWN
	nop
#endif /* defined(MT7620_ASIC_BOARD) */

WAIT_SDRAM_INIT_DOWN:
	la		t5, SDRAM_CFG1_REG
	lw t6, 0(t5)
	nop
	and  	t6, t6, SDRAM_CFG1_SDRAM_INIT_DONE
	beqz	t6, WAIT_SDRAM_INIT_DOWN
	nop

#endif // ON_BOARD_SDR //

SDRAM_INIT_DOWN:
#endif /* #if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000) */

#if defined(RT3883_FPGA_BOARD) || defined(RT3883_ASIC_BOARD)
#ifdef ON_BOARD_DDR2
#if (TEXT_BASE != 0xBFC00000) && (TEXT_BASE != 0xBF000000) && (TEXT_BASE != 0xBC000000)
        /* DDR initialization: reg SYSCFG1[25:16]: 
	 * ODT disabled, LVCMOS=1, half drive, turn ON RT3662 DDR IO ODT as 150 ohm when read DRAM
	 */
        li t1, RALINK_SYSCTL_BASE + 0x14
	lw t2, 0(t1)
	nop
	and t2, ~(0x3FF<<16)
	or t2, (0x361<<16)
	sw t2, 0(t1)
	nop
#endif
#endif
#endif


#ifdef RT3352_ASIC_BOARD

	/* adjust the SW reg voltage level higher */
            li t1, RALINK_SYSCTL_BASE + 0x88
	    li t2, 0xECC340
	    sw t2, 0(t1)
            nop
	
	/* set LDODIG 1.24V */
            li t1, RALINK_SYSCTL_BASE + 0x8c
	    li t2, 0x9B82
	    sw t2, 0(t1)
            nop
#if 1
	/* RT3352 EVB board with 32bits DDR shall disable this */
	/* 
	 * Enable spreading spectrum clock 
	 * SSC_MODUMAG=7: +/-1.00% for center; -2.00% for down
	 */
	    li t1, RALINK_SYSCTL_BASE + 0x54
	    li t2, 0x71
	    nop
	    sw t2, 0(t1)
#endif

#ifdef ON_BOARD_DDR2
#if 0
	/* RT3883 EVB board with Nanya 1G DDR shall enable this */
	/* data output (DQ) delay */
	li t1, RALINK_MEMCTRL_BASE + 0x60
	li t2, 0
	sw t2, 0(t1)
	nop
	li t1, RALINK_MEMCTRL_BASE + 0x64
	li t2, 0
	sw t2, 0(t1)
	nop
#endif
#if 0
	/* RT3352 EVB board with 32bits DDR shall enable this */
	/* data output (DQ) delay */
	li t1, RALINK_MEMCTRL_BASE + 0x60
	li t2, 0xffffffff
	sw t2, 0(t1)
	nop
	li t1, RALINK_MEMCTRL_BASE + 0x64
	li t2, 0xffffffff
	sw t2, 0(t1)
	nop
#endif
#if 0
	/* RT3352 EVB board with 16/32 bits DDR shall enable this */
	/*
	 * DDR_PAD_DRV_1=00 (full drive)
	 * DDR_PAD_DS=0 (DDR2 differential RX application)
	 * DDR_PAD_LVCMO=0 (DDR default)
	 * DDR_PAD_DRV_0=00 (full drive)
	 */
	li t1, RALINK_SYSCTL_BASE + 0x14
	and t2, ~(0x33F00000) 
	sw t2, 0(t1)
	nop
#endif
#endif /* ON_BOARD_DDR2 */

#endif /* RT3352_ASIC_BOARD */


#if defined(ON_BOARD_DDR1) || defined(ON_BOARD_DDR2)
#if defined(RT3883_FPGA_BOARD) || defined(RT3883_ASIC_BOARD)
	/* get cpu frequency from SYSCFG0 bit 9:8, and adjust tRFC accordingly
	 */
	li	t0, RALINK_SYSCTL_BASE + 0x10
	lw	t1, 0(t0)
	nop
	and	t1, (0x3 << 8)
	bne	t1, (0x3 << 8), tRFC480
	nop

	/* DDR initialization: DDR_CFG0: adjust tRFC according to size and cpu clock
	 *      for a better performance
	 *	applied for both rom and ram version (SPI and NAND flash)
	 */
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	li t4, 0x2498E4F0
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t4, 0x2498E4F0
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t4, 0x2498E4F0
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t4, 0x249924F0
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t4, 0x249964F0
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	li t4, 0x249924F0
#else
	DRAM Component not defined
#endif
	j tRFCinit
	nop
tRFC480:
	bne	t1, (0x2 << 8), tRFC250
	nop
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	li t4, 0x2498E4C0
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t4, 0x2498E4C0
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t4, 0x2498E4C0
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t4, 0x249924C0
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t4, 0x249964C0
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	li t4, 0x249924C0
#else
	DRAM Component not defined
#endif
	j tRFCinit
	nop
tRFC250:
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	li t4, 0x2498A3B0
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t4, 0x2498A3B0
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t4, 0x2498A3B0
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t4, 0x2499C3B0
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t4, 0x249903B0
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	li t4, 0x2499A3B0
#else
	DRAM Component not defined
#endif

#elif defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD)

#if defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t4, 0x2498E400
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t4, 0x24992400
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t4, 0x24996400
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	li t4, 0x249A2400
#else
	DRAM Component not defined
#endif

#elif defined(RT6855_ASIC_BOARD) || defined (MT7620_ASIC_BOARD) ||  defined (MT7628_ASIC_BOARD)
#if defined(ON_BOARD_DDR1)
	li t4, 0x24218618	//use aggressive cfg 	//0x35A26500
#elif defined(ON_BOARD_DDR2)	
	//li t4, 0x35A28410
#if defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t4, 0x2419E2E5
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t4, 0x249AA2E5
#elif defined (ON_BOARD_1024M_DRAM_COMPONENT)
	li t4, 0x249B42E5
#elif defined (ON_BOARD_2048M_DRAM_COMPONENT)
	li t4, 0x249CE2E5
#else
	DRAM Component not defined
#endif

#endif

#elif defined(RT6855_FPGA_BOARD) || defined (MT7620_FPGA_BOARD) || defined (MT7628_FPGA_BOARD)
#if defined(ON_BOARD_DDR1)
	/* below 0x21086140 is base on CLK = 40Mhz setting */
        li t4, 0x21086140
#elif defined(ON_BOARD_DDR2)
/* below 0x21090138 is base on CLK = 40Mhz setting */
        li t4, 0x21090138
#endif
#elif defined(RT6855A_FPGA_BOARD) || defined(RT6855A_ASIC_BOARD)

#else
error	"DRAM Component not defined"
#endif

tRFCinit:
#if 0
	li t3, RALINK_MEMCTRL_BASE + 0x40
	sw t4, 0(t3)
	nop
#endif


#if defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD) ||\
	defined(MT7620_FPGA_BOARD) || defined(MT7620_ASIC_BOARD) || \
	defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
#if defined(RALINK_DDR_POWERSAVE)
	/* DDR: enable self auto refresh for power saving
	 * enable it by default for both RAM and ROM version (for CoC)
	 */
#if defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
	li	t0, RALINK_MEMCTRL_BASE + 0x14
#else
	li	t0, RALINK_MEMCTRL_BASE + 0x1C
#endif
	lw	t1, 0(t0)
	nop
	and	t1, 0xff000000
	or	t1, 0x01
	sw	t1, 0(t0)
	nop
#if defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
	li	t0, RALINK_MEMCTRL_BASE + 0x10
#else
	li	t0, RALINK_MEMCTRL_BASE + 0x18
#endif
	lw	t1, 0(t0)
	nop
	or	t1, 0x10
	sw	t1, 0(t0)
	nop
#endif
#endif // defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD) //
#else // SDR //
#if defined (RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD) || \
	defined (MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD) || \
	defined (MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
#if defined(RALINK_SDR_POWERSAVE)
	/* SDR:enable precharge power saving
	 */
#if defined(MT7628_FPGA_BOARD) || defined(MT7628_ASIC_BOARD)
	li	t0, RALINK_MEMCTRL_BASE + 0x14
#else
	li	t0, RALINK_MEMCTRL_BASE + 0x1C
#endif
	lw	t1, 0(t0)
	nop
	and	t1, 0xff000000
	or	t1, 0x01
	sw	t1, 0(t0)
	nop
	li	t0, RALINK_MEMCTRL_BASE + 0x04
	lw	t1, 0(t0)
	nop
	or	t1, 0x10000000
	sw	t1, 0(t0)
	nop
#endif // RALINK_MEMORY_POWER_SAVE  //
#endif  // defined(RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD)
#endif /* defined(ON_BOARD_DDR1)||defined(ON_BOARD_DDR2) */
#if defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
#if defined (RALINK_CPU_AUTOFREQUENCY)
	/* auto freq adjustment 3352,5350 support
	 */
	li	t0, RALINK_SYSCTL_BASE + 0x44
	li	t1, 0x1f0112
	sw	t1, 0(t0)
	nop
	li	t0, RALINK_SYSCTL_BASE + 0x3c
	li	t1, 0x3040101
	sw	t1, 0(t0)
	nop
	li	t0, RALINK_SYSCTL_BASE + 0x40
	li	t1, 0x80035f41
	sw	t1, 0(t0)
	nop

#endif
#endif  // defined(RT3352_FPGA_BOARD) || defined (RT3352_ASIC_BOARD) //
	//gpiomode for non 6855A
#if !defined(RT6855A_FPGA_BOARD) && !defined(RT6855A_ASIC_BOARD) && !defined(MT7628_ASIC_BOARD)
	li t5, RALINK_SYSCTL_BASE + 0x0060
	lw t6, 0(t5)
	nop
	or t6, 0x03

#if defined (RT2880_ASIC_BOARD) || defined (RT2880_FPGA_BOARD)
	/* enable normal function i2c, spi, uartl, jtag, mdio, sdram */
	and t6, ~(0x1<<0)
	and t6, ~(0x1<<2)
	and t6, ~(0x1<<3)
	and t6, ~(0x1<<4)
	and t6, ~(0x1<<5)
	and t6, ~(0x1<<6)
#elif  defined(MT7628_FPGA_BOARD) || defined (MT7628_ASIC_BOARD)
	/* Need add code for MT7628IC GPIO mode */ 
#else
	/* enable normal function i2c, spi, uartl, jtag, mdio, ge1 */
	and t6, ~(0xf<<7)
	and t6, ~(0x3<<5)
	and t6, ~(0x3)
#if defined(MT7620_FPGA_BOARD) || defined (MT7620_ASIC_BOARD)
	and t6, ~(0xf<<16)
	/* set PERST_N to GPIO MODE */
	or	t6, (0x2<<16)
	/* set NAND|SD to GPIO MODE */
	or  t6, (0x2<<18)
#else	
	/* LNA_G_SHARE_MODE and LNA_A_SHARE_MODE at normal function, not GPIO mode */
	and t6, ~(0xf<<16)
#endif
#endif

#if defined(RT3052_ASIC_BOARD) || defined(RT3352_ASIC_BOARD) || defined(RT6855_ASIC_BOARD)
#if defined(P5_MAC_TO_PHY_MODE)
	//set mdio pin to normal mode
	and t6, ~0x80
#else
	//set mdio pin to gpio mode
	or t6, 0x80
#endif

#if defined(ON_BOARD_16BIT_DRAM_BUS)
	//set SDRAM pin to gpio mode
	or t6, 0x100
#endif
#if defined(UARTF_AT_GPIO_FUNC)
	//configure UARTF pin to gpio mode (GPIO7~GPIO14)
	or t6, 0x1c
#endif
#endif

#if defined(MT7620_ASIC_BOARD)
#if defined(P5_MAC_TO_PHY_MODE)
	//set mdio pin to normal mode
	and t6, ~(0x2<<7)
#else
	//set mdio pin to gpio mode
	or t6, (0x2<<7)
#endif	
#if defined(UARTF_AT_GPIO_FUNC)
	//configure UARTF pin to gpio mode (GPIO7~GPIO14)
	or t6, 0x1c
#endif
#endif	

#ifdef MAC_TO_VITESSE_MODE
	//set spi pin to normal mode
#if defined (RT2880_FGPA_BOARD) || defined (RT2880_ASIC_BOARD)
	and t6, ~(1<<2)
#else
	and t6, ~(1<<1)
#endif
#endif
#ifdef PCI_AT_GPIO_FUNC
	or t6, 1<<7
#endif

#if defined(RT3883_FPGA_BOARD) || defined(RT3883_ASIC_BOARD)
	//PCI share mode for NOR flash read/write
#if 0
	//old PCI share mode: 3'b010
	and t6, ~(7<<11)
	or t6, 2<<11
#else
	//new PCI share mode: 3'b011
	and t6, ~(7<<11)
	or t6, 3<<11
#endif

#endif

	//set GPIOMODE
	nop
	sw t6, 0(t5)
	nop

#ifdef PCI_AT_GPIO_FUNC
	/* output high */
	li t5, 0xa0300674
	li t6, 0xffffffff
	nop
	sw t6,0(t5)
	nop

	li t5, 0xa0300670
	li t6, 0xffffffff
	nop
	sw t6, 0(t5)
	nop
#endif
	/* SPI_HOLD should not set as GPIO output */	
	//set all GPIO to output high
        li t5, RALINK_PIO_BASE + 0x24
#if defined(MT7620_ASIC_BOARD) || defined(MT7620_FPGA_BOARD)
	/* for MT7620 RFB power saving, change UARTF pins to input mode */
        li t6, 0x00ff807f
#else
        li t6, 0xffffbfff
#endif
        nop
        sw t6, 0(t5)
        nop
        li t5, RALINK_PIO_BASE + 0x2C
        li t6, 0xffffffff
        nop
        sw t6, 0(t5)
        nop
		
#if defined(RT3052_ASIC_BOARD) || defined (RT3052_FPGA_BOARD)
#if defined(ON_BOARD_16BIT_DRAM_BUS)
	//if sdram bus is 16bits,set gpio24~gpio39 to output high
	li t6, 0xffff
	nop
        sw t6, 0(t5)
        nop
        li t5, RALINK_PIO_BASE + 0x54
	li t6, 0xffff
	nop
        sw t6, 0(t5)
        nop
#endif
#endif

#if defined(RT5350_ASIC_BOARD)
	// set default LED polarity value for RT5350 REF board
	// Active status:
	// EPHY_LED0  H: Light
	// EPHY_LED1  H: Light
	// EPHY_LED2  H: Light
	// EPHY_LED3  L: Light
	// EPHY_LED4  H: Light

	li t5, RALINK_ETH_SW_BASE + 0x168
	li t6, 0x17
	nop
	sw t6, 0(t5)
	nop

	// set default LED polarity value for RT5350 REF board
	li t5, RALINK_11N_MAC_BASE + 0x102c
	li t6, 0x40000000
	nop
	sw t6, 0(t5)
	nop

#endif		

#if defined(RT2880_ASIC_BOARD)
	//turn on power LED (GPIO 12)
	li t5, RALINK_PIO_BASE + 0x24
	lw t6, 0(t5)
	nop
	or t6, 1<<12
	sw t6, 0(t5)
	nop
	li t5, RALINK_PIO_BASE + 0x30
	li t6, 1<<12
	nop
	sw t6, 0(t5)
	nop
#elif defined(RT2883_ASIC_BOARD)
	//turn on power LED (GPIO 8)
	li t5, RALINK_PIO_BASE + 0x24
	lw t6, 0(t5)
	nop
	or t6, 1<<8
	sw t6, 0(t5)
	nop
	li t5, RALINK_PIO_BASE + 0x30
	li t6, 1<<8
	nop
	sw t6, 0(t5)
	nop
#elif defined(RT3052_ASIC_BOARD)
	//turn on power LED (GPIO 9)
	li t5, RALINK_PIO_BASE + 0x24
	lw t6, 0(t5)
	nop
	or t6, 1<<9
	sw t6, 0(t5)
	nop
	li t5, RALINK_PIO_BASE + 0x30
	li t6, 1<<9
	nop
	sw t6, 0(t5)
	nop
#elif defined(RT3352_ASIC_BOARD)
	//turn on power LED (GPIO ?)
#elif defined(RT5350_ASIC_BOARD)
	//turn on power LED (GPIO ?)
#elif defined(RT6855_ASIC_BOARD) || defined(MT7620_ASIC_BOARD)
	//turn on power LED (GPIO ?)
#elif defined(RT3883_ASIC_BOARD)
	//turn on power LED (GPIO ?)
#endif
#endif

	/* config SYSCFG or SYSCFG1 register accordingly
	 */
#if defined(RT2880_ASIC_BOARD) || defined(RT2880_FPGA_BOARD)
	// Need to remap the vector memory to 0x0 if no memory there
	li	t0, RALINK_SYSCTL_BASE + 0x0010
	li	t1, 0x00C10084 //prefetch off 
	
	sw	t1, 0(t0)
#endif
#if defined(RT2883_ASIC_BOARD) || defined(RT2883_FPGA_BOARD)
	//set PCIe to RC mode
	li	t0, RALINK_SYSCTL_BASE + 0x10
	lw	t1, 0(t0)
	nop
	or	t1, t1, (1 << 23)
	sw	t1, 0(t0)
        nop
#endif
#if defined(RT3883_ASIC_BOARD) || defined(RT3883_FPGA_BOARD)
	//FIXME: read from SYSCFG
	li	t0, RALINK_SYSCTL_BASE + 0x14
	lw	t2, 0(t0)
	nop
	and	t2, ~(3 << 14)	//GE2 to RGMII mode
	and	t2, ~(3 << 12)	//GE1 to RGMII mode
	or	t2, (1 << 8)	//PCIe to RC mode (for ethernet)
	or	t2, (1 << 7)	//PCI to Host mode (for ethernet)
	sw	t2, 0(t0)
	nop
#endif


#if defined(RT2880_FPGA_BOARD) || defined(RT2880_ASIC_BOARD)
	li	t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG

	/* Initialize caches...
	 */
	bal	mips_cache_reset
	nop

	/* ... and enable them.  
	 */
	li	t0, CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG
#endif

	/* Set up temporary stack.
	 */
	li	a0, CFG_INIT_SP_OFFSET
	//bal	mips_cache_lock
	nop

	li	t0, CFG_SDRAM_BASE + CFG_INIT_SP_OFFSET
	la	sp, 0(t0)

 	/* Initialize GOT pointer.
	 */
#if 0
	bal	1f
	nop
	.word	_GLOBAL_OFFSET_TABLE_ - 1f + 4
1:
	move	gp, ra
	lw	t1, 0(ra)
	add	gp, t1
#else
	/* winfred: a easier way to get gp value so that mipsel-linux-as can
	 *   assemble correctly without -mips_allow_branch_to_undefined flag
	 */
	bal	1f
	nop
        .word	_GLOBAL_OFFSET_TABLE_
1:
	lw	gp, 0(ra)
#endif
#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)
#if defined (CONFIG_DDR_CAL)
	bal		lock_dcache
	nop
	bal		fill_icache
	nop
	li		sp, 0x89000000+0x2F00
#endif	
#endif	
	la	t9, board_init_f
	j	t9
	nop

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * a0 = addr_sp
 * a1 = gd
 * a2 = destination address
 */
	.globl	relocate_code
	.ent	relocate_code
relocate_code:
#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)
#if defined (CONFIG_DDR_CAL)
	bal		unlock_dcache
	nop
#endif
#endif
	move	sp, a0		/* Set new stack pointer		*/

	li	t0, CFG_MONITOR_BASE
	la	t3, in_ram
	lw	t2, -12(t3)	/* t2 <-- uboot_end_data	*/
	move	t1, a2

	/*
	 * Fix GOT pointer:
	 *
	 * New GOT-PTR = (old GOT-PTR - CFG_MONITOR_BASE) + Destination Address
	 */
	move	t6, gp
	sub	gp, CFG_MONITOR_BASE
	add	gp, a2			/* gp now adjusted		*/
	sub	t6, gp, t6		/* t6 <-- relocation offset	*/

	/*
	 * t0 = source address
	 * t1 = target address
	 * t2 = source end address
	 */
	/* On the purple board we copy the code earlier in a special way
	 * in order to solve flash problems
	 */
#ifndef CONFIG_PURPLE
1:
	lw	t3, 0(t0)
	sw	t3, 0(t1)
	addu	t0, 4
	ble	t0, t2, 1b
	addu	t1, 4			/* delay slot			*/
#endif

	/* If caches were enabled, we would have to flush them here.
	 */

	/* Jump to where we've relocated ourselves.
	 */
	addi	t0, a2, in_ram - _start
	j	t0
	nop

	.word	uboot_end_data
	.word	uboot_end
	.word	num_got_entries

in_ram:
	/* Now we want to update GOT.
	 */
	lw	t3, -4(t0)	/* t3 <-- num_got_entries	*/
	addi	t4, gp, 8	/* Skipping first two entries.	*/
	li	t2, 2
1:
	lw	t1, 0(t4)
	beqz	t1, 2f
	add	t1, t6
	sw	t1, 0(t4)
2:
	addi	t2, 1
	blt	t2, t3, 1b
	addi	t4, 4		/* delay slot			*/

	/* Clear BSS.
	 */
	lw	t1, -12(t0)	/* t1 <-- uboot_end_data	*/
	lw	t2, -8(t0)	/* t2 <-- uboot_end		*/
	add	t1, t6		/* adjust pointers		*/
	add	t2, t6

	sub	t1, 4
1:	addi	t1, 4
	bltl	t1, t2, 1b
	sw	zero, 0(t1)	/* delay slot			*/

	move	a0, a1
	la	t9, board_init_r
	j	t9
	move	a1, a2		/* delay slot			*/

	.end	relocate_code


	/* Exception handlers.
	 */
romReserved:
	b romReserved

romExcHandle:
	b romExcHandle
#if defined(RT6855A_ASIC_BOARD)
	.global         rt6855A_cpu_pll
	.ent            rt6855A_cpu_pll
rt6855A_cpu_pll:
	la              t0, RALINK_SYSCTL_BASE + 0x8C
	lw              t1, 0(t0)
	nop
	srl             t2, t1, 9
	andi            t2, t2, 0x1
	bnez            t2, TFBGA_PACKAGE_DOWN
	nop
	srl             t2, t1, 26
	andi            t2, t2, 0x1
	beqz            t2, LQFP_PACKAGE_DOWN
	nop
LQFP_PACKAGE_UP:
	/* 500Mhz up to 560Mhz */
	la              t0, RALINK_SYSCTL_BASE+0x1D0
	addiu           t3, zero, 100-1         /* t3=DIVF */
	li              t4, 2<<8                /* t4=DIVQ */
	//addiu           t6, zero, 0           /* t6=DIVR */
	li              t6, ~((0x0FF<<16)|(0x7<<8)|0x1F)
1:
	lw              t8, 0(t0)
	nop
	and             t8, t8, t6
	or              t8, t8, t4
	sll             t7, t3, 16
	or              t8, t8, t7
	sw              t8, 0(t0)
	nop
	/* wait at least 50 usec for PLL lock */
	li              t5, ((50*(500+50))/3)
3:
	subu            t5, t5, 1
	bgtz            t5, 3b
	nop

	addiu           t3, t3, 1
	addiu           t7, zero, 112-1
	subu            t8, t7, t3
	bgez            t8, 1b
	nop
	j               PLL_DONE
	nop
LQFP_PACKAGE_DOWN:
	/* 500Mhz down to 420Mhz */
	la              t0, RALINK_SYSCTL_BASE+0x1D0
	addiu           t3, zero, 100-1         /* t3=DIVF */
	li              t4, 2<<8                /* t4=DIVQ */
	//addiu           t6, zero, 0             /* t6=DIVR */
	li              t6, ~((0x0FF<<16)|(0x7<<8)|0x1F)
1:
	lw              t8, 0(t0)
	nop
	and             t8, t8, t6
	or              t8, t8, t4
	sll             t7, t3, 16
	or              t8, t8, t7
	sw              t8, 0(t0)
	nop

	/* wait at least 50 usec for PLL lock */
	li              t5, ((50*(500+50))/3)
3:
	subu            t5, t5, 1
	bgtz            t5, 3b
	nop

	subu            t3, t3, 1
	addiu           t7, zero, 84-1
	subu            t8, t3, t7
	bgez            t8, 1b
	nop
	j               PLL_DONE
	nop
TFBGA_PACKAGE_DOWN:
	/* 665Mhz down to 560Mhz */
	la              t0, RALINK_SYSCTL_BASE+0x1CC
	addiu           t3, zero, 133-1         /* t3=DIVF */
	li              t4, 1<<8                /* t4=DIVQ */
	addiu           t6, zero, 1             /* t6=DIVR */
	li              t1, ~((0x0FF<<16)|(0x7<<8)|0x1F)

1:
	lw              t8, 0(t0)
	nop
	and             t8, t8, t1
	or              t8, t8, t4
	or              t8, t8, t6
	sll             t7, t3, 16
	or              t8, t8, t7
	sw              t8, 0(t0)
	nop

	/* wait at least 50 usec for PLL lock */
	li              t5, ((50*(700+70))/3)
3:
	subu            t5, t5, 1
	bgtz            t5, 3b
	nop

	subu            t3, t3, 1
	addiu           t7, zero, 120-1
	subu            t8, t3, t7
	bgez            t8, 1b
	nop
	addiu           t3, zero, 238-1
	li              t4, 2<<8

2:
	lw              t8, 0(t0)
	nop
	and             t8, t8, t1
	or              t8, t2, t4
	or              t8, t8, t6
	sll             t7, t3, 16
	or              t8, t8, t7
	sw              t8, 0(t0)
	nop
	/* wait at least 50 usec for PLL lock */
	li              t5, ((50*(600+60))/3)
4:
	subu            t5, t5, 1
	bgtz            t5, 4b
	nop

	subu            t3, t3, 1*2
	addiu           t7, zero, 224-1
	subu            t8, t3, t7
	bgez            t8, 2b
	nop
PLL_DONE:
	jr              ra
	nop
	.end            rt6855A_cpu_pll
#endif
#if defined(MT7620_ASIC_BOARD) || defined(MT7620_FPGA_BOARD) || defined(MT7628_ASIC_BOARD) || defined(MT7628_FPGA_BOARD)
/************************************************************************************/
/*																					*/	
/*	void init_cpu_pll(const u16 cpll_param)											*/
/*	cpll_param = a0 = AUX1[12],AUX0[11],MULTI_RATIO[10:8],DIV_RATIO[7:6],SSC[5:0]   */
/*																					*/
/************************************************************************************/
	.globl		init_cpu_pll
	.ent		init_cpu_pll
init_cpu_pll:
	/* check AUX1[12] and AUX0[11] */
	la			t0, RALINK_CPLLCFG1_REG
	lw			t1, 0(t0)
	li			t2, 1<<12
	and			t2, a0, t2
	move		t5, zero
	li			t4, 1<<25			/* t4=1<<25, AUX1 as clock source */
	bnez		t2, 1f
	nop
	li			t3, 1<<11
	and			t3, a0, t3
	li			t4, 1<<24			/* t4=1<<24, AUX0 as clock source */
	movz		t4, zero, t3
1:	
	/* Switch clock source from CPU PLL to AUX1 */
	la			t0, RALINK_CPLLCFG1_REG
	lw			t1, 0(t0)
	li			t2, ~(0x3<<24)
	and			t1, t1, t2
	//li			t2, 1<<25			/* choose AUX1 XTAL 20/40Mhz for backup clock source */
	li			t2, 1<<24				/* choose 480Mhz for backup clock source */
	movn		t2, zero, t4
	or			t1, t1, t2
	or			t1, t1, t4
	sw			t1, 0(t0)	
	bnez		t4, CPLL_EXIT 	
	nop
	
	/* read CPLLCFG0 before SW_CFG = 1 */
	la          t0, RALINK_CPLLCFG0_REG
	lw          t1, 0(t0)

	/* apply new params */
	la          t0, RALINK_CPLLCFG0_REG
	lw          t3, 0(t0)
	li          t2, 1<<31
	or          t3, t3, t2
	sw          t3, 0(t0)
	nop
	/* Power down CPU PLL */
	la			t0, RALINK_CPLLCFG1_REG
	lw			t2, 0(t0)
	li			t3, 1<<26
	or			t2, t2, t3
	sw			t2, 0(t0)
	/* Set new CPU PLL freq & SSC */
	//la			t0, RALINK_CPLLCFG0_REG
	//lw			t1, 0(t0)
	and			t2, a0, 0x3F
	sll			t2, t2, 4
	srl			t3, a0, 6
	and			t3, t3, 0x3
   	sll			t3, t3, 10	
	srl			t4, a0, 8
	and			t4, t4, 0x7
	sll			t4, t4, 16
	or			t2, t2, t3
	or			t2, t2, t4
	li			t3, ~((0x7<<16)|(0x3<<10)|(0x1F<<4))
	and			t1, t1, t3
	or			t1, t1, t2
	li			t5, 1<<31
	or			t1, t1, t5
#if 0	
	la			t2, RALINK_SYSCFG0_REG
	lw			t3, 0(t0)
	addiu		t4, zero, 0x1
	sll			t3, t3, 6
	and			t3, t3, t4
	li			t5, ~(1<<15)
	and			t1, t1, t5
	li			t5, 1<<15
	movz		t5,	zero, t3			/* t3=0, XTAL=20Mhz */
	or			t1, t1, t5
#endif
	la          t0, RALINK_CPLLCFG0_REG
	sw			t1, 0(t0)
	nop
	/* Power up CPU PLL */
	la			t0, RALINK_CPLLCFG1_REG
	lw			t1, 0(t0)
	li			t2, ~(1<<26)
	and			t1, t1, t2
	sw			t1, 0(t0)
	nop

	/* Polling CPU PLL PD Signal */
	la			t0, RALINK_CPLLCFG1_REG
	li			t2, 1<<23	
1:
	lw			t1, 0(t0)
	nop	
	and			t1, t1, t2
  beqz		t1, 1b
	nop	

	/* Switch clock source from CPU_CLK_AUX0/AUX1 to CPU PLL */
	la			t0, RALINK_CPLLCFG1_REG
	lw			t1, 0(t0)
	li			t2, ~(0x3<<24)
	and			t1, t1, t2
	sw			t1, 0(t0)
	nop
CPLL_EXIT:
	jr			ra
	.end		init_cpu_pll
/**********************************************************/	
#if defined(MT7620_ASIC_BOARD) || defined(MT7628_ASIC_BOARD)
		.text
		.global SDR_CFG0_TBL
		.align  3
SDR_CFG0_TBL:
					/* 150MHZ */    	/* 120MHZ */
		.word   	0x12A263A3,     	0x12825282
		.word   	0x12A263A3,     	0x12825282
		.word   	0x129263A3,     	0x12825282
		.word   	0x11A263A3,     	0x11825282
		.text
		.global SDR_CFG1_TBL
		.align  3
SDR_CFG1_TBL:	
					/* 150MHZ */    	/* 120MHZ */
		.word   	0xF80108E8,     	0xF801073F      /* 8MB */
		.word  		0xF81108E8,     	0xF811073F      /* 16MB */
		.word   	0xF8120475,     	0xF812039F      /* 32MB */
		.word   	0xF8220475,     	0xF822039F      /* 64MB */
		
		.text
		.global	DDR1_CFG0_TBL
		.align	3
DDR1_CFG0_TBL:
					/* 200MHZ */		/* 160MHZ */
        .word       0x34A1EB94,         0x239964A1		/* 8MB */
		.word       0x34A1EB94,         0x239964A1		/* 16MB */
		.word		0x34A1E5CA, 		0x239964A1		/* 32MB */
		.word		0x3421E5CA, 		0x239984A1		/* 64MB */
		.word		0x241B05CA, 		0x239AB4A1		/* 128MB */	

		
		.text
		.global	DDR1_CFG1_TBL
		.align	3
DDR1_CFG1_TBL:
					/* 200MHZ */		/* 160MHZ */
		.word       0x20262324,         0x20262323		/* 8MB */
		.word       0x202A2324,         0x202A2323		/* 16MB */
		.word		0x202E2324, 		0x202E2323		/* 32MB */
		.word		0x20322324, 		0x20322323		/* 64MB */
		.word		0x20362334, 		0x20362333		/* 128MB */	
		
		.text
		.align	3
					/* 200MHZ */		/* 160MHZ */
DDR1_CFG2_TBL:
		.word       0x28000033,         0x00000033
		.word       0x28000033,         0x00000033		
		.word		0x28000033, 		0x00000033
		.word		0x28000033, 		0x00000033
		.word		0x28000033, 		0x00000033
			
DDR1_CFG3_TBL:
		.word       0x00000002,         0x00000002
		.word       0x00000002,         0x00000002
		.word		0x00000002, 		0x00000002
		.word		0x00000002, 		0x00000002
		.word		0x00000002, 		0x00000002
DDR1_CFG4_TBL:
		.word       0x00000000,         0x00000000
		.word       0x00000000,         0x00000000
		.word		0x00000000, 		0x00000000
		.word		0x00000000, 		0x00000000
		.word		0x00000000, 		0x00000000

		.text
		.global	DDR2_CFG0_TBL
		.align	3
DDR2_CFG0_TBL:
					/* 200MHZ */		/* 160MHZ */
		.word		0x2519E2E5, 		0x23918250			/* 32MB */
		.word		0x249AA2E5, 		0x239A2250			/* 64MB */
		.word		0x249B42E5,		 	0x2392A250			/* 128MB */
		.word		0x249CE2E5,			0x24140250			/* 256MB */
		
		.text
		.global	DDR2_CFG1_TBL
		.align	3
DDR2_CFG1_TBL:
					/* 200MHZ */		/* 160MHZ */
		.word		0x222E2323,		 	0x222E2322			/* 32MB */
		.word		0x22322323,		 	0x22322322			/* 64MB */
		.word		0x22362323,		 	0x22362322			/* 128MB */
		.word		0x223A2323,		 	0x223A2322			/* 256MB */
		
		.text
		.align	3
					/* 200MHZ */		/* 160MHZ */
DDR2_CFG2_TBL:
		.word		0x68000C43, 		0x40000A43
		.word		0x68000C43, 		0x40000A43
		.word		0x68000C43, 		0x40000A43
		.word		0x68000C43, 		0x40000A43
		
		.text
		.align	3
DDR2_CFG3_TBL:
#if defined(MT7628_ASIC_BOARD)
					/* 200MHZ */        /* 160MHZ */
		.word       0x00000452,         0x00000452
		.word       0x00000452,         0x00000452
		.word       0x00000452,         0x00000452
		.word       0x00000452,         0x00000452

#else
					/* 200MHZ */		/* 160MHZ */
		.word		0x00000416, 		0x00000416
		.word		0x00000416, 		0x00000416
		.word		0x00000416, 		0x00000416
		.word		0x00000416, 		0x00000416
#endif		
		.text
		.align	3
DDR2_CFG4_TBL:
					/* 200MHZ */		/* 160MHZ */
		.word		0x0000000A, 		0x00000006
		.word		0x0000000A, 		0x00000008
		.word		0x0000000A, 		0x00000008
		.word		0x0000000A, 		0x00000008
#endif	
/**********************************************************/	
#endif
#if defined(RT6855A_ASIC_BOARD)
		.text
		.global SDR_CFG0_TBL
		.align  3
SDR_CFG0_TBL:
			/* 140MHZ */    /* 105MHZ */
		.word   0x11925282,     0x11623161
		.text
		.global SDR_CFG1_TBL
		.align  3
SDR_CFG1_TBL:	
			/* 140MHZ */    /* 105MHZ */
		.word   0x8000088B,     0x80000668      /* 2MB */
		.word   0x8001088B,     0x80010668      /* 8MB */
		.word   0x8011088B,     0x80110668      /* 16MB */
		.word   0x80120445,     0x80120334      /* 32MB */

		.text
		.global	DDR1_CFG0_TBL
		.align	3
DDR1_CFG0_TBL:
			/* 233MHZ */	/* 175MHZ */	       /* 166MHZ */	       /* 125MHZ */
		.word	0x352A2E34, 	0x3421AAAF,		0x23998A20,		0x2319279E	/* 8MB */
		.word	0x352A2E34, 	0x3421AAAF,		0x23998A20,		0x2319279E	/* 16MB */
		.word	0x352A271A, 	0x3421A558,		0x2399850E,		0x231923CF	/* 32MB */
		.word	0x352A271A, 	0x3421A558,		0x2399850E,		0x231923CF	/* 64MB */
		
		.text
		.global	DDR1_CFG1_TBL
		.align	3
DDR1_CFG1_TBL:
			/* 233MHZ */	/* 175MHZ */	       /* 166MHZ */	       /* 125MHZ */
		.word	0x20262424, 	0x20262324,		0x20262323,		0x20262223	/* 8MB */
		.word	0x202A2424, 	0x202A2324,		0x202A2323,		0x202A2223	/* 16MB */
		.word	0x202E2424, 	0x202E2324,		0x202E2323,		0x202E2223	/* 32MB */
		.word	0x20322424, 	0x20322324,		0x20322323,		0x20322223	/* 64MB */
		
		.text
		.align	3
			/* 233MHZ */	/* 175MHZ */	        /* 166MHZ */	       /* 125MHZ */
DDR1_CFG2_TBL:
		.word	0x00000033, 	0x00000033,		0x00000033,		0x00000063	
DDR1_CFG3_TBL:
		.word	0x00000000, 	0x00000000,		0x00000000,		0x00000002
DDR1_CFG4_TBL:
		.word	0x00000000, 	0x00000000,		0x00000000,		0x00000000
				
		.text
		.global	DDR2_CFG0_TBL
		.align	3
DDR2_CFG0_TBL:
			/* 233MHZ */	/* 175MHZ */	        /* 166MHZ */    	/* 125MHZ */
		.word	0x35A2438D, 	0x2419C2AB,		0x2419A287,		0x231941E7	/* 32MB */
		.word	0x35A3238D, 	0x241A62AB,		0x241A4287,		0x2319C1E7	/* 64MB */
		.word	0x35A3C38D, 	0x241AE2AB,		0x241AC287,		0x231A01E7	/* 128MB */
		.word	0x35A5E38D, 	0x241C62AB,		0x241C2287,		0x231B21E7	/* 256MB */
		
		.text
		.global	DDR2_CFG1_TBL
		.align	3
DDR2_CFG1_TBL:
			/* 233MHZ */	/* 175MHZ */	        /* 166MHZ */	        /* 125MHZ */
		.word	0x222E2424, 	0x222E2324,		0x222E2324,		0x212E2223	/* 32MB */
		.word	0x22322424, 	0x22322324,		0x22322324,		0x21322223	/* 64MB */
		.word	0x22362424, 	0x22362324,		0x22362324,		0x21362223	/* 128MB */
		.word	0x223A2424, 	0x223A2324,		0x223A2324,		0x213A2223	/* 256MB */
		
		.text
		.align	3
			/* 233MHZ */	/* 175MHZ */	       /* 166MHZ */	        /* 125MHZ */
DDR2_CFG2_TBL:
		.word	0x68000E43, 	0x40000C43,		0x40000C43,		0x40000843
DDR2_CFG3_TBL:
		.word	0x10000400, 	0x00000400,		0x00000402,		0x00000402
DDR2_CFG4_TBL:
		.word	0x0010080C, 	0x0000000B,		0x0000000B,		0x0000000B
#endif	

#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)
#if defined (CONFIG_DDR_CAL)
		/* === lock d$ === */	
		.globl	lock_dcache
		.ent	lock_dcache
lock_dcache:
		lui	$14, 0x8900		# Get a KSeg0 address for cacheops
		move	$12, $14
		li		$10, 4*2
2:	
		# Set TagLo registers
		li	$13, 0x1FFFF800
		and	$13, $13, $12
		ori	$13, $13, (0x1<<5)|(0x1<<7)
		mtc0	$13, $28, 2
		move	$9, $12
		li	$15, 64
		li	$11, 32
		# Index Store Tag Cache Op
		# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0x9, 0($9)
		add	$15, -1			# Decrement set counter
		bne	$15, $0, 1b
		add	$9, $11		# Get next line address
				
		addiu	$12, 0x800
		li  $13, 0x1FFFF800
		and $13, $13, $12
		ori $13, $13, (0x1<<5)|(0x1<<7)
		mtc0    $13, $28, 2

		subu	$10, 1
		bgtz	$10, 2b
		nop
		jr		ra
		nop
		jr		ra
		nop
		.end	lock_dcache
		
		/* === unlock d$ === */	
		.globl	unlock_dcache
		.ent	unlock_dcache
unlock_dcache:
		lui	$14, 0x8000		# Get a KSeg0 address for cacheops
		move	$12, $14
		li		$10, 4*2
2:	
		# Clear TagLo/TagHi registers
		mtc0	$0, $28, 2
		move	$9, $12
		li	$15, 64
		li	$11, 32
		# Index Store Tag Cache Op
		# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0x9, 0($9)
		add	$15, -1			# Decrement set counter
		bne	$15, $0, 1b
		add	$9, $11		# Get next line address		
		
		subu	$10, 1
		bgtz	$10, 2b
		nop
		jr		ra
		nop
		
		.end	unlock_dcache

		/* === fill I$ === */			
		.globl	fill_icache
		.ent	fill_icache
fill_icache:
		lui		$14, %hi(dram_cali)		# Get a KSeg0 address for cacheops
		addiu	$14, %lo(dram_cali)

		li	$13, 0xDFFFFFFF
		and	$13, $13, $14
		li	$15, 512
		li	$11, 32
		# Fill Cache Op
1:	cache	0x14, 0($13)
		add	$15, -1			# Decrement set counter
		nop
		bne	$15, $0, 1b
		add	$13, $11		# Get next line address		

		jr		ra
		nop
		.end	fill_icache
#endif
#endif
