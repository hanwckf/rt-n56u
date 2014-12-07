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

#define RT2880_LED_1 0x2
#define RT2880_LED_2 0x4
#define RT2880_LED_3 0x8
#define RT2880_LED_4 0x10
#define RT2880_LED_5 0x20
#define RT2880_LED_6 0x40
#define RT2880_LED_7 0x80

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

#if 0 //DISCARD exception_vect
	.section except_vect
	
except_vector:		
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
#endif //DISCARD exception sector

	.set noreorder
	.globl _start
	.section .text
_start:
reset:
#if defined (RT2883_FPGA_BOARD) || defined (RT3052_FPGA_BOARD) || defined (RT3052_ASIC_BOARD) || defined (RT2883_ASIC_BOARD)
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
#if 0 // YT
	# Set KSeg0 to cacheable
	# Config.K0
	mfc0	$10, $16
	li	$11, 0x7
	not	$11
	and	$10, $11
	or	$10, 0x3
	mtc0	$10, $16
#endif //
	
	# Clear Count register
	mtc0	$0, $9

	# Set compare to -1 to delay 1st count=compare
	# Also, clears timer interrupt
	li	$10, -1
	mtc0	$10, $11

#if 0	//disable cache
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
#endif //disable cache

15:

#endif	

 
/* delay cycle */	
	li t0,0xFFFF
	li t1,0x1
1:
	sub t0, t0, t1
	bnez t0, 1b
/* end of delay cycle */	

	li t5,SDRAM_CFG0_REG
	lw t6,0(t5)
	nop
	and t6,0xF0000000
	
#if defined (RT2880_FPGA_BOARD) || defined (RT2883_FPGA_BOARD) || defined (RT3052_FPGA_BOARD)
#ifdef RT2880_FPGA_BOARD
#ifdef RT2880_MP
	nop
	or t6,0x01825282
	//or t6,0x01815282
	nop	
#else /* RT2880_SHUTTLE */
	or t6,0x91825282
	//or t6,0x91815282
#endif
#else //2883, 3052 fpga
	nop
	or t6,0xD1825282
	//or t6,0x01815282
	nop	
#endif
#else //RT2880_ASIC_BOARD, RT2883_ASIC_BOARD, RT3052_ASIC_BOARD
	or t6,0xD1825272
#endif
	nop
	sw t6,0(t5)
	nop

	// justic whether SDRAM active
    	li t5,SDRAM_CFG1_REG
	lw t6,0(t5)
	nop
	and  	t6, t6, SDRAM_CFG1_SDRAM_INIT_DONE
	bnez	t6, SDRAM_INIT_DONE
	nop
	
	nop
#if defined (RT2880_ASIC_BOARD) || defined (RT2883_ASIC_BOARD) || defined (RT3052_ASIC_BOARD)
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	//64Mbits sdram component
	li t6,0xa1010600
#elif  ON_BOARD_128M_DRAM_COMPONENT 
	//128Mbits sdram component
	li t6,0xa1110600
#elif  ON_BOARD_256M_DRAM_COMPONENT 
	//256Mbits sdram component
	li t6,0xa1120600
#else
	DRAM Component not defined
#endif
#ifdef ON_BOARD_32BIT_DRAM_BUS
	and t6,0xFEFFFFFF
	or t6,(1<<24)
#elif  ON_BOARD_16BIT_DRAM_BUS
	and t6,0xFEFFFFFF
#else
	DRAM bus not defined
#endif
#else
#ifdef ON_BOARD_64M_DRAM_COMPONENT
	//64Mbits sdram component
	li t6,0x81010096
#elif  ON_BOARD_128M_DRAM_COMPONENT 
	//128Mbits sdram component
	li t6,0x81110096
#elif  ON_BOARD_256M_DRAM_COMPONENT 
	//256Mbits sdram component
	li t6,0x81120096
#else
	DRAM Component not defined
#endif
#ifdef ON_BOARD_32BIT_DRAM_BUS
	and t6,0xFEFFFFFF
	or t6,(1<<24)
#elif  ON_BOARD_16BIT_DRAM_BUS
	and t6,0xFEFFFFFF
#else
	DRAM bus not defined
#endif
#endif
	nop
	sw t6,0(t5)
	nop

	
WAIT_SDRAM_INIT_DOWN:	

	lw t6,0(t5)
	nop
	and  	t6, t6, SDRAM_CFG1_SDRAM_INIT_DONE
	beqz	t6, WAIT_SDRAM_INIT_DOWN
	nop


SDRAM_INIT_DONE:
	
	li t5,RALINK_SYSCTL_BASE + 0x0060
	li t6,0x3
#if defined(RT3052_ASIC_BOARD)
#if defined(P5_MAC_TO_PHY_MODE)
	//set mdio pin to normal mode
	and t6,~0x80
#else
	//set mdio pin to gpio mode
	or t6,0x80
#endif

	//configure UARTF pin to gpio mode (GPIO7~GPIO14)
#if defined(UARTF_AT_GPIO_FUNC)
	or t6,0x1c
#endif

#endif
#ifdef MAC_TO_VITESSE_MODE
	and t6,~(1<<2)
#endif
#ifdef PCI_AT_GPIO_FUNC
	or t6,1<<7
#endif
	nop
	sw t6,0(t5)
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
	sw t6,0(t5)
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
#endif
#if defined(RT2880_ASIC_BOARD) || defined(RT2880_FPGA_BOARD)
   // Need to remap the vector memory to 0x0 if no memory there
	li	t0, RALINK_SYSCTL_BASE + 0x0010
	li	t1, 0x00C10084 //prefetch off 
	
	sw	t1, 0(t0)
#endif
	

#if defined(RT3052_ASIC_BOARD) || defined(RT3052_FPGA_BOARD)|| defined(RT3050_FPGA_BOARD)|| defined(RT3050_ASIC_BOARD)
	li t0,RALINK_SYSCTL_BASE + 0x10
	lw t1,0(t0)
	nop
	and t1,t1,(1 << 18)
	bne t1,zero,SYTEM_CLOCK_SET_384MHZ
	nop	

	//  Initialize Icache size to 16K 

	mfc0	t0, CP0_CONFIG
	or t0,(1<<19)
	mtc0	t0, CP0_CONFIG
	nop
	
	mfc0	t0, CP0_CONFIG,1
	move    t1 ,t0
	and     t0,~(0x7 << 22)
	or      t0,(1 <<22)
	mtc0	t0, CP0_CONFIG,1
	nop
	mfc0	t0, CP0_CONFIG
	and t0,~(1<<19)
	mtc0	t0, CP0_CONFIG
	nop
	nop
	
SYTEM_CLOCK_SET_384MHZ:	
#endif


#if defined(RT2880_FPGA_BOARD) || defined(RT2880_ASIC_BOARD)
		/* CONFIG0 register */
	li	t0, CONF_CM_UNCACHED
	mtc0	t0, CP0_CONFIG
#if 1
	
	/* Initialize caches...
	 */
	
	bal	mips_cache_reset
	nop
	
	/* ... and enable them.  
	
	#define CONF_CM_CACHABLE_NO_WA		0
	#define CONF_CM_CACHABLE_WA		1
	#define CONF_CM_UNCACHED		2
	#define CONF_CM_CACHABLE_NONCOHERENT	3
	#define CONF_CM_CACHABLE_CE		4
	
	 */
	li	t0, CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG

#endif
#endif

		
		
	/* Set up temporary stack.
	 */
	li	a0, CFG_INIT_SP_OFFSET
	//bal	mips_cache_lock
	nop
	
	

//	li	t0, CFG_SDRAM_BASE + CFG_INIT_SP_OFFSET
//	la	sp, 0(t0)

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

	// relocate got entries
	move	t4, gp				// <---- t4:	current GP
	la	t3,  _GLOBAL_OFFSET_TABLE_	// <---- t3:	original GP
	subu	t1, t4, t3			// <---- t1:	offset to relocate
	beqz	t1, toload_stage2		// <---- (t1 == 0) ?(no_relocate): (do_relocate)
	nop

	bal	num_got
	nop	
	.word	num_got_entries
num_got:		
	lw	t0, 0(ra)	//number_got_entries
	addi	t4, 8		//skip first 2 enties
	addiu	t0, -2
	blez	t0, toload_stage2
	nop

2:	lw	t2, 0(t4)
	beqz	t2, 3f
	add	t2, t1
	sw	t2, 0(t4)
	
3:	addiu	t0, -1
	bgtz	t0, 2b
	addi	t4, 4
	
toload_stage2:				
	.extern _fstack		//this reference to stag2's stack.
	la	sp, TEXT_BASE - 8
	.extern load_stage2
	la	t9, load_stage2
	jal	t9
	nop


#if 0
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
#endif //COFNIG_STAGE1
		
romReserved:
	b romReserved

romExcHandle:
	b romExcHandle
