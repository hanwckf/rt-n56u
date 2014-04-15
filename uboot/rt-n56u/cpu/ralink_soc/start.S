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

#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <rt_mmap.h>

#define SDRAM_CFG0_REG RALINK_SYSCTL_BASE + 0x0300
#define SDRAM_CFG1_REG RALINK_SYSCTL_BASE + 0x0304
#define SDRAM_CFG0_ALWAYS_ONE ( 1 << 31)
#define SDRAM_CFG1_SDRAM_INIT_START ( 1 << 31)
#define SDRAM_CFG1_SDRAM_INIT_DONE ( 1 << 30)

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
    defined (RT3883_FPGA_BOARD) || defined (RT3883_ASIC_BOARD)
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

#if (TEXT_BASE == 0xBFC00000) || (TEXT_BASE == 0xBF000000) || (TEXT_BASE == 0xBC000000)
 
	/* SDR and DDR initialization: delay 200us
	 */
	li t0, 0xFFFF
	li t1, 0x1
1:
	sub t0, t0, t1
	bnez t0, 1b
	nop

#ifdef ON_BOARD_DDR
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

	/* DDR initialization: reset pin to 0
	 */
	li t1, RALINK_SYSCTL_BASE + 0x34
	sw zero, 0(t1)
	nop

	/* DDR initialization: wait til reg DDR_CFG1 bit 21 equal to 1 (ready)
	 */
DDR_READY:
	li t1, RALINK_MEMCTRL_BASE + 0x44
	lw t0, 0(t1)
	nop
	and t2, t0, (1<<21)
	beqz t2, DDR_READY
	nop

	/* DDR initialization:
	 *   fpga: reg DDR_CFG3 -- disable DLL
	 *   asic: reg DDR_CFG3 -- ODT disabled (bit 6,2)
	 */
	li t1, RALINK_MEMCTRL_BASE + 0x4c
	lw t2, 0(t1)
#ifdef RT3883_ASIC_BOARD
	#disable ODT; reference board ok, ev board fail
	#and t2, ~(1<<6)

	#enable ODT; both ok
	or t2, (1<<6)
	and t2, ~(1<<2)
#endif
#ifdef RT3883_FPGA_BOARD
	or t2, 0x1
#endif
	sw t2, 0(t1)
	nop

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

#if defined (RT3352_FPGA_BOARD) || defined (RT3883_FPGA_BOARD)
	/* DDR initialization: DDR_CFG0 bit 12:0 (refresh interval) to 0x64
	 * Note. this may have a bad affect on efficiency if the clock rate is 40MHz
	 */
	li t1, RALINK_MEMCTRL_BASE + 0x40
	lw t2, 0(t1)
	nop
	and t2, ~(0xfff)
	or t2, 0x64
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
#ifdef ON_BOARD_DDR_WIDTH_16
	or t6, (1<<17)
	and t6, ~(1<<16)
#elif defined (ON_BOARD_DDR_WIDTH_8)
	and t6, ~(1<<17)
	or t6, (1<<16)
#else
	DDR width not defined
#endif
#ifdef ON_BOARD_32BIT_DRAM_BUS
	or t6, (1<<12)
#elif defined (ON_BOARD_16BIT_DRAM_BUS)
	and t6, ~(1<<12)
#else
	DRAM bus not defined
#endif
	li t5, RALINK_MEMCTRL_BASE + 0x44
	sw t6, 0(t5)
	nop

	j SDRAM_INIT_DOWN
	nop
#endif


SDR_INIT:
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
#else //2883, 3052, 3352, 3883, 5350 fpga
	nop
	or t6, 0xD1825272
	//or t6, 0x01815282
	nop	
#endif
#else //ASIC_BOARD
	or t6, 0xD1825272
#endif
	nop
	sw t6, 0(t5)
	nop


	li t5, SDRAM_CFG1_REG
#ifdef ASIC_BOARD
/*
 *	Turn on SDRAM RBC (BIT 29 in SDRAM_CFG1, offset 0x4) in RT3052.
 *	  RT2880 RBC bit is Reserved bit, and change the same value for RT2880 and RT3052
 *	  Original 0x81xx0600 -> 0xa1xx0600
 *		by bobtseng, 2008.7.7.
 */
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	li t6, 0xa1010600
#elif defined (ON_BOARD_128M_DRAM_COMPONENT)
	li t6, 0xa1110600
#elif defined (ON_BOARD_256M_DRAM_COMPONENT)
	li t6, 0xa1120300
#elif defined (ON_BOARD_512M_DRAM_COMPONENT)
	li t6, 0xa1220600
#elif defined (ON_BOARD_DDR)
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
#else
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
	sw t6, 0(t5)
	nop

WAIT_SDRAM_INIT_DOWN:	
	lw t6, 0(t5)
	nop
	and  	t6, t6, SDRAM_CFG1_SDRAM_INIT_DONE
	beqz	t6, WAIT_SDRAM_INIT_DOWN
	nop

SDRAM_INIT_DOWN:
#endif
#if defined(RT3883_FPGA_BOARD) || defined(RT3883_ASIC_BOARD)
#ifdef ON_BOARD_DDR
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

#if 0
	/* 
	 * Enable spreading spectrum clock 
	 * SSC_MODUMAG=7: +/-1.00% for center; -2.00% for down
	 */
	    li t1, RALINK_SYSCTL_BASE + 0x54
	    li t2, 0x71
	    sw t2, 0(t1)
	    nop
#endif

#ifdef ON_BOARD_DDR
#if 0
	/* DDR initialization:
	 *   fpga: reg DDR_CFG3 -- disable DLL
	 *   asic: reg DDR_CFG3 -- ODT disabled (bit 6,2)
	 */
	li t1, RALINK_MEMCTRL_BASE + 0x4c
	lw t2, 0(t1)
	and t2, ~(1<<6)
	sw t2, 0(t1)
	nop
#endif
#if 0
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
#endif /* ON_BOARD_DDR */

#endif /* RT3352_ASIC_BOARD */


#ifdef ON_BOARD_DDR
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
#endif // defined(RT3883_FPGA_BOARD) || defined(RT3883_ASIC_BOARD) //

tRFCinit:
#if 0
	li t3, RALINK_MEMCTRL_BASE + 0x40
	sw t4, 0(t3)
	nop
#endif

#if defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD)
#if defined(RALINK_DDR_POWERSAVE)
	/* DDR: enable self auto refresh for power saving
	 * enable it by default for both RAM and ROM version (for CoC)
	 */
	li	t0, RALINK_MEMCTRL_BASE + 0x1C
	lw	t1, 0(t0)
	nop
	and	t1, 0xff000000
	or	t1, 0x01
	sw	t1, 0(t0)
	nop
	li	t0, RALINK_MEMCTRL_BASE + 0x18
	lw	t1, 0(t0)
	nop
	or	t1, 0x10
	sw	t1, 0(t0)
	nop
#endif
#endif // defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD) //
#else // SDR //
#if defined(RT3352_FPGA_BOARD) || defined(RT3352_ASIC_BOARD) || \
    defined (RT5350_FPGA_BOARD) || defined (RT5350_ASIC_BOARD)
#if defined(RALINK_SDR_POWERSAVE)
	/* SDR:enable precharge power saving
	 */
	li	t0, RALINK_MEMCTRL_BASE + 0x1C
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
#endif // ON_BOARD_DDR //
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
#else
	/* enable normal function i2c, spi, uartl, jtag, mdio, ge1 */
	and t6, ~(0xf<<7)
	and t6, ~(0x3<<5)
	and t6, ~(0x3)
	/* LNA_G_SHARE_MODE and LNA_A_SHARE_MODE at normal function, not GPIO mode */
	and t6, ~(0xf<<16)
#endif

#if defined(RT3052_ASIC_BOARD) || defined(RT3352_ASIC_BOARD)
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
	//set all GPIO to output high
        li t5, RALINK_PIO_BASE + 0x24
        li t6, 0xffffbfff
        nop
        sw t6, 0(t5)
        nop
        li t5, RALINK_PIO_BASE + 0x2C
        li t6, 0xffffffff
        nop
        sw t6, 0(t5)
        nop
#if defined(ON_BOARD_16BIT_DRAM_BUS)
	//if sdram bus is 16bits,set gpio24~gpio39 to output high
        li t5, RALINK_PIO_BASE + 0x4C
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
#elif defined(RT3883_ASIC_BOARD)
	//turn on power LED (GPIO ?)
	//turn on power LED (GPIO 0) for ASUS RT-N56U
	li t5, RALINK_PIO_BASE + 0x24
	lw t6, 0(t5)
	nop
	or t6, 1<<0
	sw t6, 0(t5)
	nop
	li t5, RALINK_PIO_BASE + 0x30
	li t6, 1<<0
	nop
	sw t6, 0(t5)
	nop
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
