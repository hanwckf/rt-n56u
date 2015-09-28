#include <config.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <rt_mmap.h>
#include <sysdefs.h>
#include <ArchDefs.h>
#include <launch.h>

#define CODE_ALIGN      .align  2

#define LEAF(name)\
    .##text;\
    .##globl    name;\
    .##ent  name;\
name:

#define END(name)\
    .##size name,.-name;\
    .##end  name

#define CONFIG_SYS_SDRAM_BASE           0x80000000      /* Cached addr */
#define CONFIG_SYS_INIT_SP_OFFSET       CFG_INIT_SP_OFFSET
#define CONFIG_SYS_MONITOR_BASE		TEXT_BASE

#define WAITCODE_IN_RAM		0xA0000D00

#define GIC_SHARED_OFS      0xBFBC0000
#define GIC_SH_WEDGE        (GIC_SHARED_OFS | 0x0280)
#define GIC_LOCAL_OFS       (GIC_SHARED_OFS | 0x8000)

#define GCR_CONFIG          0xbfbf8000
#define GCR_GIC_BASE        0xbfbf8080
#define GCR_GIC_BASE_VALUE  0x1fbc0000
#define GCR_GIC_STATUS      0xbfbf80d0

#define GCR_CPC_BASE        0xbfbf8088
#define GCR_CPC_STATUS      0xbfbf80f0

#define MALTA_DISP_ADDR     0xbf000410
#define STACK_BASE_ADDR     CONFIG_SYS_SDRAM_BASE + CFG_INIT_SP_OFFSET  /* fixme: Base on memory size. */
#define STACK_SIZE_LOG2     22          /* 4Mbytes each */
#define CPC_GLOBAL_OFS      0xbfbf0000                       /* CPC base address */
#define GCR_CPC_BASE_VALUE  0x1fbf0000                       /* CPC base address value */

#define GCR_REG0_BASE_VALUE 0x1c000000                       /* CM region 0 base address value */
#define GCR_REG1_BASE_VALUE 0x60000000                       /* CM region 1 base address value */
#define GCR_REG2_BASE_VALUE 0x1c000000                       /* CM region 2 base address value */
#define GCR_REG3_BASE_VALUE 0x1c000000                       /* CM region 3 base address value */
#define GCR_REG0_MASK_VALUE 0x0000fc00                       /* CM region 0 mask value 64M */
#define GCR_REG1_MASK_VALUE 0x0000f000                       /* CM region 1 mask value 256M */
#define GCR_REG2_MASK_VALUE 0x0000fc00                       /* CM region 2 mask value 64M */
#define GCR_REG3_MASK_VALUE 0x0000fc00                       /* CM region 3 mask value 64M */

#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f     ;           \
	li k0,bev

	.set noreorder
	.globl _start
	.text
_start:
	RVECENT(reset,0)			# U-boot entry point
	RVECENT(reset,1)			# software reboot
	RVECENT(romReserved,2)
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
	XVECENT(romExcHandle,0x200)	# bfc00200: R4000 tlbmiss vector
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
	XVECENT(romExcHandle,0x280)	# bfc00280: R4000 xtlbmiss vector
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
	XVECENT(romExcHandle,0x300)	# bfc00300: R4000 cache vector
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
	XVECENT(romExcHandle,0x380)	# bfc00380: R4000 general vector
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

	/*
	 * We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */
	.align 4
reset:
    b	__reset_vector
	nop

/**************************************************************************************
 Register use while executing in this file: ("GLOBAL" denotes a common value.)
**************************************************************************************/

#define r1_all_ones     $1   /* Will hold 0xffffffff to simplify bit insertion of 1's. GLOBAL! */

#define r2_has_mt_ase   $2   /* Core implements the MT ASE. */
#define r3_is_cps       $3   /* Core is part of a Coherent Processing System. */

#define r4_temp_data    $4   /* scratch, eventually the 1st param for main (a0.) */
#define r5_temp_addr    $5   /* scratch, eventually the 2nd param for main (a1.) */
#define r6_temp_dest    $6   /* scratch, eventually the 3rd param for main (a2.) */
#define r7_temp_mark    $7   /* scratch, eventually the 4th param for main (a3.) */

#define r16_core_num    $16  /* Core number. Only core 0 is active after reset. */
#define r17_vpe_num     $17  /* MT ASE VPE number that this TC is bound to (0 if non-MT.) */
#define r18_tc_num      $18  /* MT ASE TC number (0 if non-MT.) */
#define r19_more_cores  $19  /* Number of cores in CPS addition to core 0. GLOBAL! */
#define r20_more_vpes   $20  /* Number of vpes in this core in addition to vpe 0. */
#define r21_more_tcs    $21  /* Number of tcs in vpe in addition to the first. */ 
#define r22_gcr_addr    $22  /* Uncached (kseg1) base address of the Global Config Registers. */
#define r23_cpu_num     $23  /* Unique per vpe "cpu" identifier (CP0 EBase[CPUNUM]). */
#define r24_malta_word  $24  /* Uncached (kseg1) base address of Malta ascii display. GLOBAL! */
#define r25_coreid      $25  /* Copy of cp0 PRiD GLOBAL! */

#define r26_int_addr    $26  /* Interrupt handler scratch address. */
#define r27_int_data    $27  /* Interrupt handler scratch data. */

#define r28_global_addr $28  /* Common Address of shared/coherent globals. GLOBAL! */
#define r29_stack_addr  $29  /* Unique per vpe stack pointer. */
#define r30_cpc_addr    $30  /* Address of CPC register block after cpc_init. 0 indicates no CPC. */
#define r31_return_addr $31  /* Return address for linked branches. */

.macro set_tag TAG_X
#ifdef USE_PIO_DBG
	.set noat
	la		r5_temp_addr, RALINK_PIO_BASE
	lw		r4_temp_data, 0x20(r5_temp_addr)
	li		r7_temp_mark, ~((0x7<<6)|(0x1))
	and		r4_temp_data, r4_temp_data, r7_temp_mark
	li		r7_temp_mark, ((((\TAG_X)>>1)<<6)|((\TAG_X)&0x1))
	or		r4_temp_data, r4_temp_data, r7_temp_mark
	sw		r4_temp_data, 0x20(r5_temp_addr)
	.set at
#endif	
	.endm

/**************************************************************************************
    R E S E T   E X C E P T I O N   H A N D L E R
**************************************************************************************/
	.set	noreorder           # Don't allow the assembler to reorder instructions.
	.set    noat                # Don't allow the assembler to use r1(at) for synthetic instr.

LEAF(__reset_vector)
    b       check_nmi               # Note: Real systems might want to save/dump full context.
    mtc0    $0, $9              # Clear cp0 Count (Used to measure boot time.)

    # Note: adding code here may conflict with Malta board ID register at 0xbfc0010.

END(__reset_vector)

/**************************************************************************************
    B O O T   E X C E P T I O N   H A N D L E R S
**************************************************************************************/
check_nmi: # Verify we are here due to a reset (and not NMI.)
#if 0 //example code if you need to use LED (GPIO0) to debug some issue.
    la	t0, 0xbe000600
    li	t9, 1
    sw	t9, 0(t0) /* output */
    sw	t9, 0x40(t0) /* low */
#endif
	
#ifdef USE_PIO_DBG
	la		r5_temp_addr, RALINK_PIO_BASE
	lw		r4_temp_data, 0(r5_temp_addr)
	li		r7_temp_mark, ~(0x1<<5)
	and		r4_temp_data, r4_temp_data, r7_temp_mark
	li		r7_temp_mark, (0x7<<6)|(0x1)
	or		r4_temp_data, r4_temp_data, r7_temp_mark	//output
	sw		r4_temp_data, 0(r5_temp_addr)
	lw		r4_temp_data, 0x10(r5_temp_addr)
	li		r7_temp_mark, ~((0x7<<6)|0x1)
	and		r4_temp_data, r4_temp_data, r7_temp_mark	//not invert
	sw		r4_temp_data, 0x10(r5_temp_addr)
	set_tag 0x0
	// USE GPIO0, 6,7,8
	la		r5_temp_addr, RALINK_GPIOMODE_REG
	lw		r4_temp_data, 0(r5_temp_addr)
	li		r7_temp_mark, ~(0x3<<3)	//~(0x3<<18)
	and		r4_temp_data, r4_temp_data, r7_temp_mark
	sw		r4_temp_data, 0(r5_temp_addr)
#endif	

	set_tag 0x01

#if 0	// set GPIO19(PERST_N) to output mode and pull low
	li	t0, 0xbe000600
	lw	t1, 0(t0)
	li	t2, 1<<19
	or	t1, t1, t2
	sw	t1, 0(t0)
	li	t0, 0xbe000620
	lw	t1, 0(t0)
	li	t2, ~(1<<19)
	and	t1, t1, t2
	sw	t1, 0(t0)
#endif
    mfc0    r4_temp_data, $12                   # Read CP0 Status
    srl     r4_temp_data, 19                    # Shift [NMI] into LSBs.
    andi    r4_temp_data, r4_temp_data, 1       # Inspect CP0 Config[AT]
    beqz    r4_temp_data, verify_isa            # Branch if this is NOT an NMI exception.
    nop
    sdbbp                                       # Failed assertion: not NMI.

verify_isa: # Verify device ISA meets code requirements (MIPS32 r2 or later.)
    mfc0    r4_temp_data, $16                   # Read CP0 Config
    srl     r4_temp_data, 10                    # Shift [AT AR] into LSBs.
    andi    r7_temp_mark, r4_temp_data, 0x18    # Inspect CP0 Config[AT]
    beqz    r7_temp_mark, is_mips32             # Branch if executing on MIPS32 ISA.
    andi    r7_temp_mark, r4_temp_data, 0x07    # Inspect CP0 Config[AR]
    sdbbp                                       # Failed assertion: mips32.

is_mips32:
    bnez    r7_temp_mark, init_vpe_resources    # Continue if ISA is MIPS32r2 or later.
    nop
    sdbbp                                       # Failed assertion mips32r2.

/**************************************************************************************
 What is initialized on execution depends on the core/vpe executing it.
 (A non-MT device is treated as tc0/vpe0, non-CMP device is treated as core0.)
**************************************************************************************/
init_vpe_resources:     # Every "cpu"(vpe) initializes per-vpe resources.
    bal     init_gpr    # Fill register file with dummy value then boot info.
    nop
    bal     init_cp0    # Init CP0 Status, Count, Compare, Watch*, and Cause.
    nop
    bal     init_tlb    # Generate unique EntryHi contents per entry pair.
    nop
    bal     init_gic    # Configure the global interrupt controller. 
    nop
    bnez    r17_vpe_num, init_done # If we are not a vpe0 then we are done.
    nop
    bnez    r16_core_num, init_core_resources # Only core0/vpe0 needs to init systems resources.
    nop

	set_tag 0x02

init_sys_resources:     # We are core0 vpe0.
    bal     init_cpc    # Initialize the CPS CPC (Cluster Power Controller.)
    nop
    bal     init_cm     # Initialize the CPS CM (Coherency Manager.)
    nop
    bal     init_mc     # Initialize the ROC-it2 MC (Memory Controller.)
    nop
    bal     init_l23u   # Initialize the unified L2 and L3 caches (if CCA Override is not available.)
    nop
#if 0 //MTK: not used
    bal     copy_c2_ram # Copy "C" code and data to RAM and zero bss (uncached.)
    nop
#endif
    bal     release_mp  # Release other cores to execute this boot code.
    nop

	set_tag 0x03

init_core_resources:    # We are a vpe0.
    bal     init_icache # Initialize the L1 instruction cache. (Executing using I$ on return.)
    nop
	set_tag 0x04   
    bal     init_dcache # Initialize the L1 data cache
    nop
    bal     init_l23c   # Initialize the unified L2 and L3 caches (if CCA Override is available).
    nop
#if 0 // MTK: no thread
    bal     init_itc    # Initialize Inter-Thread Communications unit
    nop
#endif
    bal     join_domain # Join the coherent domain. (OK to use D$ on return.)
    nop

#ifdef RALINK_DUAL_VPE_FUN
    bal     init_vpe1   # Set up MT ASE vpe1 to execute this boot code also.
    nop
#endif


init_done:
	set_tag 0x05
#if 0 //ignore main
    # Prepare for eret to main (sp and gp set up per vpe in init_gpr).
    la      r31_return_addr, all_done       # If main return then go to all_done:.
    la      r5_temp_addr, main
    mtc0    r5_temp_addr, $30               # ErrorEPC

    # Prepare arguments for main()
    move    r4_temp_data, r23_cpu_num      # main(arg0) is the "cpu" number (cp0 EBase[CPUNUM].)
    move    r5_temp_addr, r16_core_num     # main(arg1) is the core number.
    move    r6_temp_dest, r17_vpe_num      # main(arg2) is the vpe number.
    addiu   r7_temp_mark, r20_more_vpes, 1 # main(arg3) is the number of vpe on this core.

    eret    # Exit reset exception handler for this vpe and start execution of main().
#endif

/**************************************************************************************
**************************************************************************************/
all_done:
notmtcapable:
	/*
	 * MIPSCMP
	 * Only Core0 carries on from here
	 * Everybody else waits...
	 */
	beqz	r23_cpu_num,finish_initialisation
	nop

othercores:
 	/* FIXME any other per-CPU initialisation required? */

	li	t0,KSEG0(CPULAUNCH)
	sll	t1,r23_cpu_num, LOG2CPULAUNCH
	addu	t0,t1

	/*
	 * Set CPU online flag
	 */
	lw	t1,LAUNCH_FLAGS(t0)
	andi	t1, 0
	or	t1,LAUNCH_FREADY
	sw	t1,LAUNCH_FLAGS(t0)

	/* enable count interrupt in mask, but don't enable interrupts */
	mfc0    t2,C0_Status
	li      t1,M_StatusIM7 /* FIXME should calculate dynamically from Config3.ippti */
	or      t1,t2
	mtc0    t1,C0_Status

	li		t9, WAITCODE_IN_RAM
	jr		t9
	nop
	CODE_ALIGN

waitcode_start:
	/*
	 * Poll CPU go flag
	 */
1:
	mfc0    t1,C0_Count
	addu    t1,LAUNCHPERIOD
	mtc0    t1,C0_Compare

swwait:	/* Software wait */
	mfc0	t4,C0_Count
	subu	t4,t1
	bltz	t4,swwait
	nop
	b	checklaunch
	nop

checklaunch:
	lw	t1,LAUNCH_FLAGS(t0)
	and	t1,LAUNCH_FGO
	beqz	t1, 1b
	 nop

	/* Reset the counter and interrupts to give naive clients a chance */
	mtc0	t2,C0_Status
	mfc0	t2,C0_Count
	subu	t2,1
	mtc0	t2,C0_Compare

	/* we're off */	
	lw	t2,LAUNCH_PC(t0)
	lw	gp,LAUNCH_GP(t0)
	lw	sp,LAUNCH_SP(t0)
	lw	a0,LAUNCH_A0(t0)
	move	a1,zero
	move	a2,zero
	move	a3,zero
	ori	t1,LAUNCH_FGONE
	jr	t2
	 sw	t1,LAUNCH_FLAGS(t0)
	CODE_ALIGN
waitcode_end:


finish_initialisation:	
	/* Set up temporary stack */
	li	t0, CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET
	la	sp, 0(t0)
	set_tag 0x06
	lui	t9, %hi(board_init_f)
	addiu t9, %lo(board_init_f)
	jr	t9
	nop


/**************************************************************************************
**************************************************************************************/
init_gpr:

	# Initialize the general purpose registers and any shadow register sets.
	# Although not necessary, register initialization may be useful during boot, 
    # debug, and simulation when certain ways of initializing registers may not work
    # (xor rN, rN, rN for example.)

	# Initialize register sets
	li      $1, 0x0      # (0x0 stands out, kseg2 mapped, odd.)

	# Determine how many shadow sets are implemented (in addition to the base register set.)
	mfc0	$29, $12, 2			# C0_SRSCtl
	ext	    $30, $29, 26, 4		# S_SRSCtlHSS, W_SRSCtlHSS

next_shadow_set:
	# set PSS to shadow set to be initialized
	ins	    $29, $30, 6, 4		# S_SRSCtlPSS, W_SRSCtlPSS
	mtc0	$29, $12, 2			# C0_SRSCtl

	wrpgpr	$1, $1
	wrpgpr	$2, $1
	wrpgpr	$3, $1
	wrpgpr	$4, $1
	wrpgpr	$5, $1
	wrpgpr	$6, $1
	wrpgpr	$7, $1
	wrpgpr	$8, $1
	wrpgpr	$9, $1
	wrpgpr	$10, $1
	wrpgpr	$11, $1
	wrpgpr	$12, $1
	wrpgpr	$13, $1
	wrpgpr	$14, $1
	wrpgpr	$15, $1
	wrpgpr	$16, $1
	wrpgpr	$17, $1
	wrpgpr	$18, $1
	wrpgpr	$19, $1
	wrpgpr	$20, $1
	wrpgpr	$21, $1
	wrpgpr	$22, $1
	wrpgpr	$23, $1
	wrpgpr	$24, $1
	wrpgpr	$25, $1
	wrpgpr	$26, $1
	wrpgpr	$27, $1
	wrpgpr	$28, $1
	wrpgpr	$29, $1
    beqz    $30, set_gpr_boot_values
	wrpgpr	$30, $1
	wrpgpr	$31, $1             # Don't clobber $31 in set0. Used as r31_return_addr by bal to this code.
	b	next_shadow_set
	add	$30, -1
    
set_gpr_boot_values:
    li      r1_all_ones, 0xffffffff             # Simplify code and improve clarity
    mfc0    r4_temp_data, $15, 1                # Read CP0 EBase
	ext	    r23_cpu_num, r4_temp_data, 0, 4     # Extract CPUNum
    li      r24_malta_word, MALTA_DISP_ADDR     # Need for reporting failed assertions.
    lui     r28_global_addr, %hi(_gp)                # All vpe share globals.
	addiu	r28_global_addr, %lo(_gp)
    li      r29_stack_addr, STACK_BASE_ADDR     # Each vpe gets it's own stack.
    ins     r29_stack_addr, r23_cpu_num, STACK_SIZE_LOG2, 3

check_mt_ase:
	mfc0	r4_temp_data, $16, 1		        # C0_Config1
	bgez	r4_temp_data, no_mt_ase             # No Config2 register
	mfc0	r4_temp_data, $16, 2		        # C0_Config2
	bgez	r4_temp_data, no_mt_ase             # No Config3 register
	mfc0	r4_temp_data, $16, 3		        # C0_Config3
	and	    r4_temp_data, (1 << 2)              # M_Config3MT
	beqz	r4_temp_data, no_mt_ase
    li      r2_has_mt_ase, 0

has_mt_ase:
    li      r2_has_mt_ase, 1

    # Every vpe will set up the following to simplify resource initialization.
    mfc0    r4_temp_data, $2, 2                # Read CP0 TCBind
	ext	    r17_vpe_num, r4_temp_data,  0, 4   # Extract CurVPE
	ext	    r18_tc_num, r4_temp_data, 21, 8    # Extract CurTC
	mfc0    r4_temp_data, $0, 2		           # C0_MVPConf0
    ext	    r21_more_tcs, r4_temp_data, 0, 8   # S_MVPConf0PTC, W_MVPConf0PTC (Not used by all vpe.)
    b       check_cps
	ext	    r20_more_vpes, r4_temp_data, 10, 4 # S_MVPConf0PVPE, W_MVPConf0PVPE (Not used by all vpe.)

no_mt_ase: # This processor does not implement the MIPS32 MT ASE. Set up defaults.
	li 	    r17_vpe_num, 0
	li 	    r18_tc_num, 0
    li 	    r20_more_vpes, 0
    li 	    r21_more_tcs, 0

check_cps: # Determine if there is a coherency manager present. (Implementation Dependent.)

    mfc0    r25_coreid, $15, 0                  # CP0 PRId.
    ext     r4_temp_data, r25_coreid, 8, 16     # Extract Manuf and Core.
    li      r7_temp_mark, 0x0199                # MIPS, 1004K
    beq     r7_temp_mark, r4_temp_data, is_cps
    li      r7_temp_mark, 0x019a                # MIPS, 1074K
    beq     r7_temp_mark, r4_temp_data, is_cps
    nop

is_not_cps: # This processor is not part of a Coherent Processing System. Set up valid defaults.
	li      r3_is_cps, 0
    li      r16_core_num, 0
    b       done_init_gpr
	li 	    r19_more_cores, 0

is_cps:
	li      r3_is_cps, 1

	//MTK: access 1fbf8008 will cause excption??
    # Verify that we can find the GCRs.
	li	    r5_temp_addr, GCR_CONFIG           # KSEG1(GCRBASE)
	lw	    r4_temp_data, 0x0008(r5_temp_addr) # GCR_BASE
	ins     r5_temp_addr, $0, 29, 3            # Convert KSEG1 to physical address.
	ins	    r4_temp_data, $0, 0, 15            # Isolate physical base address of GCR.
	beq	    r5_temp_addr, r4_temp_data, gcr_found
    nop
    sdbbp   # Can't find GCR. RTL config override of MIPS default?

gcr_found:
    # Every vpe will set up the following to simplify resource initialization.
    li      r22_gcr_addr, GCR_CONFIG
    lw      r16_core_num, 0x2028(r22_gcr_addr) # Load GCR_CL_ID
    lw      r4_temp_data, 0(r22_gcr_addr)      # Load GCR_CONFIG
#ifdef RALINK_DUAL_CORE_FUN
     li     r5_temp_addr, RALINK_SYSCTL_BASE
     lw     r4_temp_data, 0x000c(r5_temp_addr)  #CHIP_REV_ID
     ext    r19_more_cores, r4_temp_data, 17, 1
#else
     li     r19_more_cores, 0
#endif

done_init_gpr:
    jr      r31_return_addr
    nop


/**************************************************************************************
**************************************************************************************/
init_cp0:

    # Initialize Status
	li	    $11, 0x00400404	# (M_StatusIM | M_StatusERL | M_StatusBEV)
	mtc0	$11, $12		# C0_Status

	# Initialize Watch registers if implemented.
	mfc0	$10, $16, 1		# C0_Config1
	ext	    $11, $10, 3, 1	# S_Config1WP, W_Config1WP
	beq	    $11, $0, done_wr
	li      $11, 0x7		# (M_WatchHiI | M_WatchHiR | M_WatchHiW)

	# Clear Watch Status bits and disable watch exceptions
	mtc0	$11, $19		# C0_WatchHi0
	mfc0	$10, $19		# C0_WatchHi0
	bgez    $10, done_wr
	mtc0	$0, $18			# C0_WatchLo0

	mtc0	$11, $19, 1		# C0_WatchHi1
	mfc0	$10, $19, 1		# C0_WatchHi1
	bgez    $10, done_wr
	mtc0	$0, $18, 1		# C0_WatchLo1

	mtc0	$11, $19, 2		# C0_WatchHi2
	mfc0	$10, $19, 2		# C0_WatchHi2
	bgez    $10, done_wr
	mtc0	$0, $18, 2		# C0_WatchLo2

	mtc0	$11, $19, 3		# C0_WatchHi3
	mfc0	$10, $19, 3		# C0_WatchHi3
	bgez    $10, done_wr
	mtc0	$0, $18, 3		# C0_WatchLo3

	mtc0	$11, $19, 4		# C0_WatchHi4
	mfc0	$10, $19, 4		# C0_WatchHi4
	bgez    $10, done_wr
	mtc0	$0, $18, 4		# C0_WatchLo4

	mtc0	$11, $19, 5		# C0_WatchHi5
	mfc0	$10, $19, 5		# C0_WatchHi5
	bgez    $10, done_wr
	mtc0	$0, $18, 5		# C0_WatchLo5

	mtc0	$11, $19, 6		# C0_WatchHi6
	mfc0	$10, $19, 6		# C0_WatchHi6
	bgez    $10, done_wr
	mtc0	$0, $18, 6		# C0_WatchLo6

	mtc0	$11, $19, 7		# C0_WatchHi7
	mtc0	$0, $18, 7		# C0_WatchLo7

done_wr:

	# Clear WP bit to avoid watch exception upon user code entry, IV, and software interrupts.
	mtc0	$0, $13			# C0_Cause: Init AFTER init of CP0 WatchHi/Lo registers.

	# Clear timer interrupt. (Count was cleared at the reset vector to allow timing boot.)
	mtc0	$0, $11		    # C0_Compare

    # Set CCA for kseg0 to cacheable (Do not access D$ on CPS untill all cores join coherent domain.)
	mfc0	$10, $16		# C0_Config
    beqz    r3_is_cps, set_kseg0_cca
	li	    $11, 3 //3			# K_CacheAttrC 
	li	    $11, 5			# K_CacheAttrCCS 
                            # Cacheable, write-back, write-allocate, coherent, read misses request Shared
set_kseg0_cca:
    ins	    $10, $11, 0, 3	# S_ConfigK0, W_ConfigK0
	mtc0	$10, $16		# C0_Config

	jr      r31_return_addr
	nop

/**************************************************************************************
**************************************************************************************/
init_tlb: # Initialize the TLB

check_for_tlb:
	# Determine if we have a TLB
	mfc0    $11, $16		# C0_Config
	ext	    $11, $11, 7, 3	# S_ConfigMT, W_ConfigMT
	li	    $15, 0x1		# K_ConfigMT_TLBMMU
	bne	    $11, $15, done_init_tlb
	mfc0    $10, $16, 1		# C0_Config1

    # Check for TLB sharing between vpe.
    beqz    r2_has_mt_ase, start_init_tlb
    nop
    beqz    r17_vpe_num, start_init_tlb
    mfc0    r4_temp_data, $0, 1                 # MVPEControl
    ext     r4_temp_data, r4_temp_data, 3, 1    # MVPEControl[STLB]
    bnez    r4_temp_data, done_init_tlb         # has MT ASE, is not vpe0, is sharing tlb so skip.
    nop


start_init_tlb:
	# Config1MMUSize == Number of TLB entries - 1
	ext     $11, $10, 25, 6	# S_Config1MMUSize, W_Config1MMUSize
	mtc0    $0, $2			# C0_EntryLo0
	mtc0    $0, $3			# C0_EntryLo1
	mtc0    $0, $5			# C0_PageMask
	mtc0    $0, $6			# C0_Wired
	li	    $12, 0x80000000

next_tlb_entry_pair:
    ins     $12, r23_cpu_num, 20, 4             # test: add "cpu" number to provide cps unique entries.
	mtc0    $11, $0			# C0_Index
	mtc0	$12, $10		# C0_EntryHi
	ehb
	tlbwi
	add	    $12, (2<<13)		# Add 8K to the address to avoid TLB conflict with previous entry

	bne	    $11, $0, next_tlb_entry_pair
	add	    $11, -1

done_init_tlb:
    jr      r31_return_addr
    nop


/**************************************************************************************
**************************************************************************************/
init_cpc:
    beqz    r3_is_cps, done_init_cpc            # Skip if non-CPS.
    nop

    lw      r4_temp_data, 0x00f0(r22_gcr_addr)  # GCR_CPC_STATUS
    andi    r4_temp_data, 1
    beqz    r4_temp_data, done_init_cpc         # Skip if CPC is not implemented.
    move    r30_cpc_addr, $0

    li      r4_temp_data, (GCR_CPC_BASE_VALUE | 0x1)   # Locate CPC at same location YAMON does.
    sw      r4_temp_data, 0x0088(r22_gcr_addr)  # GCR_CPC_BASE
    li      r30_cpc_addr, CPC_GLOBAL_OFS            # Maintain address of CPC register block.

done_init_cpc:
    jr      r31_return_addr
    nop


/**************************************************************************************
**************************************************************************************/
init_gic:

    beqz    r3_is_cps, done_gic            # Skip if non-CPS.
    nop

    li     r5_temp_addr, GCR_GIC_STATUS        # Read GCR_GIC_STATUS
	lw      r4_temp_data, 0(r5_temp_addr)
    ext     r4_temp_data, r4_temp_data, 0, 1    # Isolate  GCR_GIC_STATUS[GIC_EX].
    beqz    r4_temp_data, done_gic              # If no gic then skip.
    nop

    bnez    r23_cpu_num, init_vpe_gic           # Only core0 vpe0 inits shared portion.
    nop

    li      r5_temp_addr, GCR_GIC_BASE          # Locate and enable GIC where YAMON does.
    li      r4_temp_data, (GCR_GIC_BASE_VALUE | 1)
    sw      r4_temp_data, 0(r5_temp_addr)
    nop 

    # Verify gic is 8 "slices" of 8 interrupts giving 40 interrupts.
    li      r5_temp_addr, GIC_SHARED_OFS
    lw      r4_temp_data, 0(r5_temp_addr)       # GIC_SH_CONFIG
    ext     r4_temp_data, 16, 8                 # NUMINTERRUPTS (actually slices - 1)
    li      r7_temp_mark, 7
    beq     r4_temp_data, r7_temp_mark, configure_slices
    nop
    sdbbp   # Failed assertion that gic implements 64 external interrupts.

configure_slices:
    li      r4_temp_data, 0x00000000
    sw      r4_temp_data, 0x180(r5_temp_addr)   # GIC_SH_TRIG31_0   (Level trigger  0..5)
    li      r4_temp_data, 0x0000003F
    sw      r4_temp_data, 0x300(r5_temp_addr)   # GIC_SH_RMASK31_0  (disable        0..5)
    sw      r4_temp_data, 0x100(r5_temp_addr)   # GIC_SH_POL31_0    (Active High    0..5)
    sw      r4_temp_data, 0x380(r5_temp_addr)   # GIC_SH_SMASK31_0  (enable         0..5)

    # Hardcoded to set up the last 8 of 64 external interrupts (56..63) for IPI.
    li      r4_temp_data, 0xFF000000
    sw      r4_temp_data, 0x184(r5_temp_addr)   # GIC_SH_TRIG63_32  (edge trigger 56..63)
    sw      r4_temp_data, 0x304(r5_temp_addr)   # GIC_SH_RMASK63_32 (disable      56..63)
    sw      r4_temp_data, 0x104(r5_temp_addr)   # GIC_SH_POL63_32   (Rising Edge  56..63)
    sw      r4_temp_data, 0x384(r5_temp_addr)   # GIC_SH_SMASK63_32 (enable       56..63)

    # Initialize configuration of shared interrupts
    
    # Map interrupt source to particular pin (GIC INT6~INT31 to PIN0)
    li	    r4_temp_data, 0x80000000 //source0 to pin0
    sw      r4_temp_data, 0x500(r5_temp_addr)     # GIC_SH_MAP0_PIN
    li	    r4_temp_data, 0x80000000 //source1 to pin0
    sw      r4_temp_data, 0x504(r5_temp_addr)     # GIC_SH_MAP1_PIN
    li	    r4_temp_data, 0x80000004 //source2 to pin4
    sw      r4_temp_data, 0x508(r5_temp_addr)     # GIC_SH_MAP2_PIN
    li	    r4_temp_data, 0x80000003 //source3 to pin3
    sw      r4_temp_data, 0x50C(r5_temp_addr)     # GIC_SH_MAP3_PIN
    li	    r4_temp_data, 0x80000000 //source4 to pin0
    sw      r4_temp_data, 0x510(r5_temp_addr)     # GIC_SH_MAP4_PIN
    li	    r4_temp_data, 0x80000005 //source5 to pin5
    sw      r4_temp_data, 0x514(r5_temp_addr)     # GIC_SH_MAP5_PIN
    
    li	    r4_temp_data, 0x80000001 //source56 to pin1
    sw      r4_temp_data, 0x5E0(r5_temp_addr)     # GIC_SH_MAP56_PIN
    li	    r4_temp_data, 0x80000001 //source57 to pin1
    sw      r4_temp_data, 0x5E4(r5_temp_addr)     # GIC_SH_MAP57_PIN
    li	    r4_temp_data, 0x80000001 //source58 to pin1
    sw      r4_temp_data, 0x5E8(r5_temp_addr)     # GIC_SH_MAP58_PIN
    li	    r4_temp_data, 0x80000001 //source59 to pin1
    sw      r4_temp_data, 0x5EC(r5_temp_addr)     # GIC_SH_MAP59_PIN
    li	    r4_temp_data, 0x80000002 //source60 to pin2
    sw      r4_temp_data, 0x5F0(r5_temp_addr)     # GIC_SH_MAP60_PIN
    li	    r4_temp_data, 0x80000002 //source61 to pin2
    sw      r4_temp_data, 0x5F4(r5_temp_addr)     # GIC_SH_MAP61_PIN
    li	    r4_temp_data, 0x80000002 //source62 to pin2
    sw      r4_temp_data, 0x5F8(r5_temp_addr)     # GIC_SH_MAP62_PIN
    li	    r4_temp_data, 0x80000002 //source63 to pin2
    sw      r4_temp_data, 0x5FC(r5_temp_addr)     # GIC_SH_MAP63_PIN

    #Interrupt map to VPE (1=vpe0, 2=vpe1, 4=vpe2, 8=vpe3)
    li	    r4_temp_data, 1
    sw      r4_temp_data, 0x2000(r5_temp_addr)     # GIC_SH_MAP0_VPE31_0
    sw      r4_temp_data, 0x2020(r5_temp_addr)     # GIC_SH_MAP1_VPE31_0
    sw      r4_temp_data, 0x2040(r5_temp_addr)     # GIC_SH_MAP2_VPE31_0
    sw      r4_temp_data, 0x2060(r5_temp_addr)     # GIC_SH_MAP3_VPE31_0
    sw      r4_temp_data, 0x2080(r5_temp_addr)     # GIC_SH_MAP4_VPE31_0
    sw      r4_temp_data, 0x20A0(r5_temp_addr)     # GIC_SH_MAP5_VPE31_0
    sw      r4_temp_data, 0x20C0(r5_temp_addr)     # GIC_SH_MAP6_VPE31_0
    sw      r4_temp_data, 0x20E0(r5_temp_addr)     # GIC_SH_MAP7_VPE31_0

    sw      r4_temp_data, 0x2100(r5_temp_addr)     # GIC_SH_MAP8_VPE31_0
    sw      r4_temp_data, 0x2120(r5_temp_addr)     # GIC_SH_MAP9_VPE31_0
    sw      r4_temp_data, 0x2140(r5_temp_addr)     # GIC_SH_MAP10_VPE31_0
    sw      r4_temp_data, 0x2160(r5_temp_addr)     # GIC_SH_MAP11_VPE31_0
    sw      r4_temp_data, 0x2180(r5_temp_addr)     # GIC_SH_MAP12_VPE31_0
    sw      r4_temp_data, 0x21A0(r5_temp_addr)     # GIC_SH_MAP13_VPE31_0
    sw      r4_temp_data, 0x21C0(r5_temp_addr)     # GIC_SH_MAP14_VPE31_0
    sw      r4_temp_data, 0x21E0(r5_temp_addr)     # GIC_SH_MAP15_VPE31_0

    sw      r4_temp_data, 0x2200(r5_temp_addr)     # GIC_SH_MAP16_VPE31_0
    sw      r4_temp_data, 0x2220(r5_temp_addr)     # GIC_SH_MAP17_VPE31_0
    sw      r4_temp_data, 0x2240(r5_temp_addr)     # GIC_SH_MAP18_VPE31_0
    sw      r4_temp_data, 0x2260(r5_temp_addr)     # GIC_SH_MAP19_VPE31_0
    sw      r4_temp_data, 0x2280(r5_temp_addr)     # GIC_SH_MAP20_VPE31_0
    sw      r4_temp_data, 0x22A0(r5_temp_addr)     # GIC_SH_MAP21_VPE31_0
    sw      r4_temp_data, 0x22C0(r5_temp_addr)     # GIC_SH_MAP22_VPE31_0
    sw      r4_temp_data, 0x22E0(r5_temp_addr)     # GIC_SH_MAP23_VPE31_0

    sw      r4_temp_data, 0x2300(r5_temp_addr)     # GIC_SH_MAP24_VPE31_0
    sw      r4_temp_data, 0x2320(r5_temp_addr)     # GIC_SH_MAP25_VPE31_0
    sw      r4_temp_data, 0x2340(r5_temp_addr)     # GIC_SH_MAP26_VPE31_0
    sw      r4_temp_data, 0x2360(r5_temp_addr)     # GIC_SH_MAP27_VPE31_0
    sw      r4_temp_data, 0x2380(r5_temp_addr)     # GIC_SH_MAP28_VPE31_0
    sw      r4_temp_data, 0x23A0(r5_temp_addr)     # GIC_SH_MAP29_VPE31_0
    sw      r4_temp_data, 0x23C0(r5_temp_addr)     # GIC_SH_MAP30_VPE31_0
    sw      r4_temp_data, 0x23E0(r5_temp_addr)     # GIC_SH_MAP31_VPE31_0

    # Direct GIC_int 56..63 to vpe 0..3
    # MIPS Linux convention that last 16 interrupts implemented be set aside for IPI signaling.
    # (The actual interrupts are tied low and software sends interrupts via GIC_SH_WEDGE writes.)
    li      r4_temp_data, 1                        # vpe0 is selected for
    sw      r4_temp_data, 0x2700(r5_temp_addr)     # GIC_SH_MAP56_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe1 is selected for
    sw      r4_temp_data, 0x2720(r5_temp_addr)     # GIC_SH_MAP57_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe2 is selected for
    sw      r4_temp_data, 0x2740(r5_temp_addr)     # GIC_SH_MAP58_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe3 is selected for
    sw      r4_temp_data, 0x2760(r5_temp_addr)     # GIC_SH_MAP59_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe4 is selected for
    li      r4_temp_data, 1                        # vpe0 is selected for
    sw      r4_temp_data, 0x2780(r5_temp_addr)     # GIC_SH_MAP60_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe5 is selected for
    sw      r4_temp_data, 0x27a0(r5_temp_addr)     # GIC_SH_MAP61_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe6 is selected for
    sw      r4_temp_data, 0x27c0(r5_temp_addr)     # GIC_SH_MAP62_VPE31_0 and
    sll     r4_temp_data, r4_temp_data, 1          # vpe7 is selected for
    sw      r4_temp_data, 0x27e0(r5_temp_addr)     # GIC_SH_MAP63_VPE31_0 and
init_vpe_gic:

    # Initialize configuration of per vpe interrupts
    li      r5_temp_addr, GIC_LOCAL_OFS
    lw      r7_temp_mark, 0x0000(r5_temp_addr)     # GIC_VPEi_CFG

map_timer_int:
    ext     r4_temp_data, r7_temp_mark, 1, 1       # TIMER_ROUTABLE
    beqz    r4_temp_data, map_perfcount_int
    nop

map_perfcount_int:
    ext     r4_temp_data, r7_temp_mark, 2, 1       # PERFCOUNT_ROUTABLE
    beqz    r4_temp_data, done_gic
    nop

done_gic:
    jr      r31_return_addr
    nop

/**************************************************************************************
Hardcoded Denali Databahn DRAM controller initialization.
**************************************************************************************/
init_mc:
#ifdef UBOOT_ROM
//#define USE_PCIE_SRAM				1
#define FE_SRAM_STACK                0xBE108000
#define RALINK_CLKCFG0_REG     		(RALINK_SYSCTL_BASE+0x2C)
#define RALINK_RSTCTRL_REG     		(RALINK_SYSCTL_BASE+0x34)


	//set SPI clock to system bus /(5+2)
	li	t0, RALINK_SPI_BASE + 0x3c
	//sw	zero, 0(t0)
	li	t1, ~0x0FFF
	lw	t2, 0(t0)
	and	t2, t2, t1
	ori	t2, t2, 0x5
	sw	t2, 0(t0)

	/* change CPU ratio from 1/A to 1/1 */
	li	t0, RALINK_DYN_CFG0_REG
	li	t1, ~(0x0F<<8)
	lw	t2, 0(t0)
	and	t2, t2, t1
	li	t1, 1<<8
	or  t2, t2, t1
	sw	t2, 0(t0)

	/* enter accessible PSE SRAM */

	/* RESET PSE SRAM */
	li t0, 0xBE100004
	li	t1 ,0x1
	sw t1, 0(t0)	
	li	t2, 0x333333/3
#if 0	
DLY:
	subu t2, t2, 1
	bgtz t2, DLY
	nop
#endif	
	
	li t0, 0xBE100004
	lw t1, 0(t0)
	ori t1, 0x6  //FE_RST_GLO[2:1]=2'b11 (bit2=PSE_RAM mode, bit1=enable)
	sw t1, 0(t0)
	nop

#ifndef BYPASS_MTK_DDR_CAL
#ifdef USE_PCIE_SRAM
	/* enable accessible PCIe SRAM */
	li		t0, RALINK_RSTCTRL_REG
	li		t1, 0x7<<24
	sw		t1, 0(t0)
	li		t0, RALINK_CLKCFG0_REG
	lw		t1, 0(t0)
	li		t2, 1<<17
	or		t1, t1, t2
	sw		t1, 0(t0)
	li		t0, RALINK_RSTCTRL_REG
	sw		zero, 0(t0)	
	li		t0, 0xBE1400B0
	ori		t1, t3, 1
	sw		t1, 0(t0)
#endif	
	set_tag 0xC
	/* move code to SRAM */
	li		t0, 0xBE00001C
	lw		t1, 0(t0)
	//ori		t1, t1, 0x1
	li		t1, 0x0
	sw		t1, 0(t0)

	lui		t0, %hi(uboot_end_data)
	addiu	t0, t0, %lo(uboot_end_data)
	li		t1, 0xBE108800
	li		t3, (24*1024-256-0x800)
	
1:	
	lw		t2, 0(t0)
	sw		t2, 0(t1)
	addiu	t0, t0, 4
	addiu	t1, t1, 4
	subu	t3, t3, 4
	bgtz	t3, 1b
	nop

	li		t0, 0xBE10DFF0
	sw		r31_return_addr, 0(t0)

	li		t0, 0xBE10DF00
	sw		$1, 0(t0)
	sw		$2, 4(t0)
	sw		$3, 8(t0)
	sw		$4, 12(t0)
	sw		$5, 16(t0)
	sw		$6, 20(t0)
	sw		$7, 24(t0)
	sw		$16,28(t0)
	sw		$17, 32(t0)
	sw		$18, 36(t0)
	sw		$19, 40(t0)
	sw		$20, 44(t0)
	sw		$21, 48(t0)
	sw		$22, 52(t0)
	sw		$23, 56(t0)
	sw		$24, 60(t0)
	sw		$25, 64(t0)
	sw		$26, 68(t0)
	sw		$27, 72(t0)
	sw		$28, 76(t0)
	sw		$29, 80(t0)
	sw		$30, 84(t0)
	sw		$31, 88(t0)

	li		t9 , 0xBE108800
	jalr    t9
	nop
	li		t0, 0xBE10DFF0		
	lw		r31_return_addr, 0(t0)
	
	li		t0, 0xBE10DF00
	lw		$1, 0(t0)
	lw		$2, 4(t0)
	lw		$3, 8(t0)
	lw		$4, 12(t0)
	lw		$5, 16(t0)
	lw		$6, 20(t0)
	lw		$7, 24(t0)
	lw		$16,28(t0)
	lw		$17, 32(t0)
	lw		$18, 36(t0)
	lw		$19, 40(t0)
	lw		$20, 44(t0)
	lw		$21, 48(t0)
	lw		$22, 52(t0)
	lw		$23, 56(t0)
	lw		$24, 60(t0)
	lw		$25, 64(t0)
	lw		$26, 68(t0)
	lw		$27, 72(t0)
	lw		$28, 76(t0)
	lw		$29, 80(t0)
	lw		$30, 84(t0)
	lw		$31, 88(t0)


	set_tag 0xE
#ifdef USE_PCIE_SRAM
	/* disable accessible PCIe SRAM */
	li		t0, 0xBE1400B0
	lw		t1, 0(t0)
	li		t2, ~0x1
	and		t1, t1, t2
	sw		t1, 0(t0)
	
	li		t0, RALINK_RSTCTRL_REG
	li		t1, 0x7<<24
	sw		t1, 0(t0)
	
	li		t0, RALINK_CLKCFG0_REG
	lw		t1, 0(t0)
	li		t2, ~(1<<17)
	and		t1, t1, t2
	sw		t1, 0(t0)
	
	li		t0, RALINK_RSTCTRL_REG
	sw		zero, 0(t0)
#endif	
#else
	li      sp, FE_SRAM_STACK
	sw		r31_return_addr, 0(sp)
	addiu	sp, sp ,4
#if !defined (FPGA_BOARD)	
	/* change mpll source from CR setting */
	li		t0, 0xBE00002C
	lw		t1, 0(t0)
	li		t2, 1<<23
	or		t1, t1, t2
	sw		t1, 0(t0)
	set_tag 0xC
	lui		t9, %hi(mempll_init)
	addiu	t9, t9, %lo(mempll_init)
	jalr    t9
	nop
MEMPLL_INIT_DONE:	
#endif
	set_tag 0xD
	lui		t9, %hi(ddr_initialize)
	addiu	t9, t9, %lo(ddt_initialize)
	jalr    t9
	nop
	set_tag 0xE
DDR_INIT_DONE:
	subu	sp, sp, 4
	lw		r31_return_addr, 0(sp)
#endif
	/* exit accessible PSE SRAM */
	li t0, 0xBE100004
	lw t1, 0(t0)
	li t2, ~0x6
        and t1, t1, t2
	ori t1, 1 //reset PSE
        sw  t1, 0(t0)
#else	
// Reset PSE
#if 1
	li 	t0, 0xBE00080C
	lw	t1, 0(t0)
	srl	t1, t1, 1
	sll	t1, t1, 1
	sw	t1, 0(t0)
	
	li 	t0, 0xBE100004
	lw 	t1, 0(t0)
	li	t1, 0x1
	sw	t1, 0(t0)
#endif			
#endif // UBOOT_ROM //
COPY_WAITCODE:
	lui	t0, %hi(waitcode_start)
	addiu	t0, t0, %lo(waitcode_start)
	lui	t1, %hi(waitcode_end)
	addiu	t1, t1, %lo(waitcode_end)
	lui	t2, %hi(WAITCODE_IN_RAM)
	addiu	t2, t2, %lo(WAITCODE_IN_RAM)
1:
	lw	t3, 0(t0)
	sw	t3, 0(t2)
	addiu	t0, t0, 4
	addiu	t2, t2, 4
	bne		t0, t1, 1b
	nop
	jr   r31_return_addr
	nop
/**************************************************************************************
**************************************************************************************/
#if 0 //done by board_init_r()
copy_c2_ram:

    # Copy code and read-only/initialized data from FLASH to (uncached) RAM.
    la      r5_temp_addr, _zap1
    ins     r5_temp_addr, r1_all_ones, 29, 1
    la      r6_temp_dest, _ftext_ram
    ins     r6_temp_dest, r1_all_ones, 29, 1
    la      r7_temp_mark, _edata_ram
    ins     r7_temp_mark, r1_all_ones, 29, 1
    beq     r6_temp_dest, r7_temp_mark, zero_bss
    nop
next_ram_word:
    lw      r4_temp_data, 0(r5_temp_addr)
    sw      r4_temp_data, 0(r6_temp_dest)
    addiu   r6_temp_dest, 4
    bne     r7_temp_mark, r6_temp_dest, next_ram_word
    addiu   r5_temp_addr, 4

zero_bss:    
    la      r5_temp_addr, _fbss
    ins     r5_temp_addr, r1_all_ones, 29, 1
    la      r7_temp_mark, _end
    ins     r7_temp_mark, r1_all_ones, 29, 1
    beq     r5_temp_addr, r7_temp_mark, copy_c2_ram_done
    nop
next_bss_word:
    sw      $0, 0(r5_temp_addr)
    addiu   r5_temp_addr, 4
    bne     r5_temp_addr, r7_temp_mark, next_bss_word
    nop

copy_c2_ram_done:
    # initialize "early" global variable.
    la      r5_temp_addr, num_cores
    ins     r5_temp_addr, r1_all_ones, 29, 1 # Uncached kseg1
    add     r4_temp_data, r19_more_cores, 1
    sw      r4_temp_data, 0(r5_temp_addr)

    jr      r31_return_addr
    nop
#endif
/**************************************************************************************
**************************************************************************************/
release_mp:

    blez    r19_more_cores, done_release_mp     # If no more cores then we are done.
    li      r7_temp_mark, 1

    beqz    r30_cpc_addr, release_next_core     # If no CPC then use GCR_CO_RESET_RELEASE
    nop											# else use CPC Power Up command.

powerup_next_core:
    # Send PwrUp command to next core causing execution at their reset exception vector.
    move    r4_temp_data, r7_temp_mark
    sll     r4_temp_data, 16
    sw      r4_temp_data, 0x2010(r30_cpc_addr)  # CPC_CL_OTHER
    li      r4_temp_data, 3                     # "PwrUp" power domain command.
    sw      r4_temp_data, 0x4000(r30_cpc_addr)  # CPC_CO_CMD PwrUp
    bne     r19_more_cores, r7_temp_mark, powerup_next_core
    add     r7_temp_mark, r7_temp_mark, 1

    jr      r31_return_addr
    nop

release_next_core:
    # Release next core to execute at their reset exception vector.
    move    r4_temp_data, r7_temp_mark
    sll     r4_temp_data, 16
    sw      r4_temp_data, 0x2018(r22_gcr_addr) # GCR_CL_OTHER
    sw      $0, 0x4000(r22_gcr_addr)           # GCR_CO_RESET_RELEASE
    bne     r19_more_cores, r7_temp_mark, release_next_core
    add     r7_temp_mark, r7_temp_mark, 1

done_release_mp:
    jr      r31_return_addr
    nop

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

/**************************************************************************************
**************************************************************************************/

init_icache:

	# Can be skipped if using magic simulation cache flush

	# Determine how big the I$ is
	mfc0	$10, $16, 1		# C0_Config1

	# Isolate I$ Line Size
	ext	    $11, $10, 19, 3		# S_Config1IL, W_Config1IL

	# Skip ahead if No I$
	beq	    $11, $0, done_icache
	nop

	li	    $14, 2
	sllv    $11, $14, $11		# Now have true I$ line size in bytes

	ext	    $12, $10, 22, 3		# S_Config1IS, W_Config1IS
	li	    $14, 64
	sllv    $12, $14, $12		# I$ Sets per way

	# Config1IA == I$ Assoc - 1
	ext	    $13, $10, 16, 3		# S_Config1IA, W_Config1IA
	add	    $13, 1

	mul	    $12, $12, $13		# Total number of sets

	lui	    $14, 0x8000			# Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0    $0, $28				# C0_ITagLo
	mtc0    $0, $29				# C0_ITagHi

	move    $15, $12

next_icache_tag:
	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
	cache   0x8, 0($14)			# ICIndexStTag
	add	    $15, -1				# Decrement set counter
	bne     $15, $0, next_icache_tag
	add     $14, $11		    # Get next line address

done_icache:
	# Modify return address to kseg0 which is cacheable (for code linked in kseg1.)
#if !defined (MTK_NAND)
    ins     r31_return_addr, $0, 29, 1    
#endif    
    jr      r31_return_addr
    nop

/**************************************************************************************
**************************************************************************************/

init_dcache:

	# Isolate D$ Line Size
	ext	    $11, $10, 10, 3		# S_Config1DL, W_Config1DL

	# Skip ahead if No D$
	beq	    $11, $0, done_dcache
	nop

	li	    $14, 2
	sllv	$11, $14, $11	# Now have true D$ line size in bytes

	ext	    $12, $10, 13, 3		# S_Config1DS, W_Config1DS
	li	    $14, 64
	sllv	$12, $14, $12	# D$ Sets per way

	# Config1DA == D$ Assoc - 1
	ext	    $13, $10, 7, 3		# S_Config1DA, W_Config1DA
	add	    $13, 1

	mul	    $12, $12, $13		# Get total number of sets

	lui	    $14, 0x8000		    # Get a KSeg0 address for cacheops

	# Clear TagLo/TagHi registers
	mtc0	$0, $28			# C0_TagLo
	mtc0	$0, $29			# C0_TagHi
	mtc0	$0, $28, 2		# C0_DTagLo
	mtc0	$0, $29, 2		# C0_DTagHi

	move	$15, $12

next_dcache_tag:
	# Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit

    cache	0x9, 0($14)		# DCIndexStTag
	add	    $15, -1			    # Decrement set counter

	bne	    $15, $0, next_dcache_tag
	add	    $14, $11		    # Get next line address

done_dcache:
    jr      r31_return_addr
    nop


/**************************************************************************************
**************************************************************************************/
init_cm:

	beqz    r3_is_cps, done_cm_init     # skip if not a CPS or CM register verification failed.
	nop

	# Allow each core access to the CM registers (they should only access their local registers.)
    li     r5_temp_addr, GCR_CONFIG            # KSEG1(GCRBASE)
	li	    r4_temp_data, 2                     # Start building mask for cores in this cps.
    sll     r4_temp_data, r4_temp_data, r19_more_cores
    addiu   r4_temp_data, -1                    # Complete mask.
	sw	    r4_temp_data, 0x0020(r5_temp_addr)	# GCR_ACCESS

    # Check to see if this CPS implements an IOCU.
    lw      r4_temp_data, 0(r22_gcr_addr)       # Load GCR_CONFIG
	ext	    r4_temp_data, r4_temp_data, 8, 4    # Extract NUMIOCU.
    beqz    r4_temp_data, done_cm_init
	lui	    r4_temp_data, 0xffff

	# Disable the CM regions if there is an IOCU.
	li	    r4_temp_data, GCR_REG0_BASE_VALUE   # Physical address
        sw	    r4_temp_data, 0x0090(r5_temp_addr)	# GCR_REG0_BASE
        li	    r4_temp_data, GCR_REG1_BASE_VALUE   # Physical address
        sw	    r4_temp_data, 0x00a0(r5_temp_addr)	# GCR_REG1_BASE
        li	    r4_temp_data, GCR_REG2_BASE_VALUE   # Physical address
        sw	    r4_temp_data, 0x00b0(r5_temp_addr)	# GCR_REG2_BASE
        li	    r4_temp_data, GCR_REG3_BASE_VALUE   # Physical address
        sw	    r4_temp_data, 0x00c0(r5_temp_addr)	# GCR_REG3_BASE

        lw	    r4_temp_data, 0x0098(r5_temp_addr)	# GCR_REG0_MASK
        li          $11,          GCR_REG0_MASK_VALUE
        ins	    r4_temp_data, $11, 16, 16
        li          $11,          0x2
        ins	    r4_temp_data, $11, 0, 2
        sw	    r4_temp_data, 0x0098(r5_temp_addr)	# GCR_REG0_MASK

        lw	    r4_temp_data, 0x00a8(r5_temp_addr)	# GCR_REG1_MASK
        li          $11,          GCR_REG1_MASK_VALUE
        ins	    r4_temp_data, $11, 16, 16
        li          $11,          0x2
        ins	    r4_temp_data, $11, 0, 2
        sw	    r4_temp_data, 0x00a8(r5_temp_addr)	# GCR_REG1_MASK

        lw	    r4_temp_data, 0x00b8(r5_temp_addr)	# GCR_REG2_MASK
        li          $11,          GCR_REG2_MASK_VALUE
        ins	    r4_temp_data, $11, 16, 16
        li          $11,          0x2
        ins	    r4_temp_data, $11, 0, 2
        sw	    r4_temp_data, 0x00b8(r5_temp_addr)	# GCR_REG2_MASK

        lw	    r4_temp_data, 0x00c8(r5_temp_addr)	# GCR_REG3_MASK
	li          $11,          GCR_REG3_MASK_VALUE
        ins	    r4_temp_data, $11, 16, 16
        li          $11,          0x2
        ins	    r4_temp_data, $11, 0, 2
        sw	    r4_temp_data, 0x00c8(r5_temp_addr)	# GCR_REG3_MASK

        lw	    r4_temp_data, 0x0008(r5_temp_addr)	# GCR_BASE
        ins	    r4_temp_data, $0, 0, 2
        sw	    r4_temp_data, 0x0008(r5_temp_addr)	# GCR_BASE

        lw	    r4_temp_data, 0x0010(r5_temp_addr)	# GCR_BASE
	li	    $11,	  0x1
        ins	    r4_temp_data, $11, 16, 1
        sw	    r4_temp_data, 0x0010(r5_temp_addr)	# GCR_BASE

done_cm_init:
	jr      r31_return_addr
	nop

/**************************************************************************************
**************************************************************************************/
init_itc:
    nop
    # enhanceme: Add ITC init.
done_init_itc:
	jr      r31_return_addr
	nop

/**************************************************************************************
**************************************************************************************/
join_domain:

    beqz    r3_is_cps, done_join_domain    # If this is not a CPS then we are done.
    nop

	# Enable coherence and allow interventions from all other cores.
	# (Write access enabled via GCR_ACCESS by core 0.)
	li        $9, 1
	sll       $9, r19_more_cores
	sll       $9, 1
	addiu     $9, -1
	or        $9, (1<<4)

	sw	    $9, 0x2008(r22_gcr_addr)	# GCR_CL_COHERENCE
	ehb

	# Cores other than core 0 can relinquish write access to CM regs here.

	# enhanceme: Use iter-core ITC (1004K MR2 or 1074K) for efficient synchronization.
	# (Uncached accesses to GCR_CO_COHERENCE will flood bus but will not slow D$ init.)
    move    r7_temp_mark, $0

next_coherent_core:
    sll     r4_temp_data, r7_temp_mark, 16
    sw      r4_temp_data, 0x2018(r22_gcr_addr) # GCR_CL_OTHER[CoreNum]

busy_wait_coherent_core:
    lw      r4_temp_data, 0x4008(r22_gcr_addr) # GCR_CO_COHERENCE
    beqz    r4_temp_data, busy_wait_coherent_core   # Busy wait on cores joining.
    nop

    bne     r7_temp_mark, r19_more_cores, next_coherent_core
    addiu   r7_temp_mark, 1

done_join_domain:
	jr      r31_return_addr
	nop

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

/**************************************************************************************
**************************************************************************************/
init_l23u:

    # Use MR2 CCA Override to allow cached execution of L2/3 init.
    # Check for CCA_Override_Enable by writing a one.

    beqz    r3_is_cps, init_l23
    nop
	lw	    r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE
    li      r7_temp_mark, 0x50                  # CM_DEFAULT_TARGET Memory
    ins     r4_temp_data, r7_temp_mark, 0, 8    # CCA Override Uncached enabled
    sw      r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE. Comment to prevent use of CCA Override.
    
	lw	    r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE
    ext     r4_temp_data, r4_temp_data, 4, 1    # CCA_Override_Enable
    bnez    r4_temp_data, done_l23          	# Skip uncached execution if CCA Override is implemented.
    nop
    b		init_l23
    nop

init_l23c:

    beqz    r3_is_cps, done_l3cache
    nop
	lw	    r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE
    bnez    r16_core_num, done_l3cache			# Only done from core 0.
    ext     r4_temp_data, r4_temp_data, 4, 1    # CCA_Override_Enable
    beqz    r4_temp_data, done_l3cache          # Skip cached execution if CCA Override is not implemented.
    nop

init_l23:
	# L2 Cache initialization routine

	# Check L2 cache size
	mfc0	$10, $16, 2		# C0_Config2

	# Isolate L2$ Line Size
	ext	    $11, $10, 4, 4		# S_Config2SL, W_Config2SL

	# Skip ahead if No L2$
	beq	    $11, $0, done_l2cache
	nop

	li	    $14, 2
	sllv	$11, $14, $11		# Now have true L2$ line size in bytes

	# Isolate L2$ Sets per Way
	ext	    $12, $10, 8, 4		# S_Config2SS, W_Config2SS
	li	    $14, 64
	sllv	$12, $14, $12		# L2$ Sets per way

	# Isolate L2$ Associativity
	# L2$ Assoc (-1)
	ext	    $13, $10, 0, 4		# S_Config2SA, W_Config2SA
	add	    $13, 1

	mul	    $12, $12, $13		# Get total number of sets

	lui	    $14, 0x8000		# Get a KSeg0 address for cacheops

	# Clear L23TagLo/L23TagHi registers
	mtc0	$0, $28, 4
	mtc0	$0, $29, 4

	move	$15, $12

	# L2$ Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0xB, 0($14)		# SCIndexStTag
	add	    $15, -1			# Decrement set counter

	bne	    $15, $0, 1b
	add	    $14, $11		# Get next line address

done_l2cache:

	# Isolate L3$ Line Size
	ext	    $11, $10, 20, 4		# S_Config2TL, W_Config2TL

	# Skip ahead if No L3$
	beq	    $11, $0, done_l3cache
	nop

	li	    $14, 2
	sllv	$11, $14, $11		# Now have true L3$ line size in bytes

	# Isolate L3$ Sets per Way
	ext	    $12, $10, 24, 4		# S_Config2TS, W_Config2TS
	li	    $14, 64
	sllv	$12, $14, $12		# L2$ Sets per way

	# Isolate L3$ Associativity
	# L3$ Assoc (-1)
	ext	    $13, $10, 16, 4		# S_Config2TA, W_Config2TA
	add	    $13, 1

	mul	    $12, $12, $13		# Get total number of sets

	lui	    $14, 0x8000		    # Get a KSeg0 address for cacheops

	# Clear L23TagLo/L23TagHi registers
	mtc0	$0, $28, 4
	mtc0	$0, $29, 4

	move	$15, $12

	# L3$ Index Store Tag Cache Op
	# Will invalidate the tag entry, clear the lock bit, and clear the LRF bit
1:	cache	0xA, 0($14)		# TCIndexStTag
	add	    $15, -1			# Decrement set counter

	bne	    $15, $0, 1b
	add	    $14, $11		# Get next line address

done_l3cache:
    # disable CCA Override
    beqz    r3_is_cps, done_l23
    nop
	lw	    r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE
    ins     r4_temp_data, $0, 0, 8              # CCA Override disabled
    sw      r4_temp_data, 0x0008(r22_gcr_addr)  # GCR_BASE

done_l23:
#if 0 //MTK: configure L2 cache size
    .set at
    mfc0    t0, CP0_CONFIG
    or t0,  (1<<19)
    mtc0    t0, CP0_CONFIG                                                                  
    nop

    mfc0    t0, CP0_CONFIG,2
    move    t1 ,t0   
    and     t0,~(0xF << 8)
    or      t0,(4 <<8) //1024*32*8=256K
//  or      t0,(3 <<8) //512*32*8=128K
//  and     t0,~(0xF << 4) //no cache
    mtc0    t0, CP0_CONFIG,2
    nop

    mfc0    t0, CP0_CONFIG
    and t0, ~(1<<19)
    mtc0    t0, CP0_CONFIG
    nop
    nop
    .set noat
#endif
    jr      r31_return_addr
    nop

/**************************************************************************************
**************************************************************************************/
init_vpe1:

    # fixme: a lot of this is now redundant. each vpe init's it's own resources.
    # Initializing a vpe should ammount to setting it's lowest numberd bound tc to
    # start execution from the reset vector.
    # Each vpe will need to set up additional tc bound to it. (No rebinding.)

    beqz    r21_more_tcs, done_init_vpe1   # If there is no .
    nop
    beqz    r20_more_vpes, done_init_vpe1   # If there is no vpe1 then skip init_vpe1.
    nop

	# This is executing on TC0 bound to VPE0.  Therefore VPEConf0.MVP is set.
	# Enter config mode
	mfc0	$8, $0, 1		# C0_MVPCtl
	or	$8, (1 << 1)		# M_MVPCtlVPC
	mtc0	$8, $0, 1		# C0_MVPCtl
	ehb

#define NTCS	$10
#define NVPES	$11
#define TC	$12

	# Get number of TC's and VPE's
	mfc0	$8, $0, 2		# C0_MVPConf0
	ext	NTCS, $8, 0, 8		# S_MVPConf0PTC, W_MVPConf0PTC
	ext	NVPES, $8, 10, 4	# S_MVPConf0PVPE, W_MVPConf0PVPE

	# Initialise TC's/VPE's
	move	TC, $0
nexttc:
	# Select TCn
	mfc0	$8, $1, 1		# C0_VPECtl
	ins	$8, TC, 0, 8		# S_VPECtlTargTC, W_VPECtlTargTC
	mtc0	$8, $1, 1		# C0_VPECtl
	ehb

	# Bind TC to next VPE
	beqz	TC, nextvpe		# Don't rebind TC0
	nop

	# Halt all TC's other than TC0
	li	$8, 1			# M_TCHaltH
	mttc0	$8, $2, 4		# C0_TCHalt
	ehb

	slt	$9, NVPES, TC
	bnez	$9, 2f			# Bind spare TC's to VPElast
	move	$9, NVPES

	# Set XTC for active TC's
	mftc0	$8, $1, 2		# C0_VPEConf0
	ins	$8, TC, 21, 8		# S_VPEConf0XTC, W_VPEConf0XTC
	mttc0	$8, $1, 2		# C0_VPEConf0

	move	$9, TC
2:
	# Bind TC to a VPE
	mftc0	$8, $2, 2		# C0_TCBind
	ins	$8, $9, 0, 4		# S_TCBindCurVPE, W_TCBindCurVPE
	mttc0	$8, $2, 2		# C0_TCBind

	# Set up TCStatus register:
	# Disable Coprocessor Usable bits
	# Disable MDMX/DSP ASE
	# Clear Dirty TC
	# not dynamically allocatable
	# not allocated
	# Kernel mode
	# interrupt exempt
	# ASID 0
	li	$8, (1 << 10)		# M_TCStatusIXMT
	mttc0	$8, $2, 1		# C0_TCStatus

	# Initialize the TC's register file
	mttgpr	$0, $1
	mttgpr	$0, $2
	mttgpr	$0, $3
	mttgpr	$0, $4
	mttgpr	$0, $5
	mttgpr	$0, $6
	mttgpr	$0, $7
	mttgpr	$0, $8
	mttgpr	$0, $9
	mttgpr	$0, $10
	mttgpr	$0, $11
	mttgpr	$0, $12
	mttgpr	$0, $13
	mttgpr	$0, $14
	mttgpr	$0, $15
	mttgpr	$0, $16
	mttgpr	$0, $17
	mttgpr	$0, $18
	mttgpr	$0, $19
	mttgpr	$0, $20
	mttgpr	$0, $21
	mttgpr	$0, $22
	mttgpr	$0, $23
	mttgpr	$0, $24
	mttgpr	$0, $25
	mttgpr	$0, $26
	mttgpr	$0, $27
	mttgpr	$0, $28
	mttgpr	$0, $29
	mttgpr	$0, $30
	mttgpr	$0, $31

nextvpe:
	slt	$9, NVPES, TC
	bnez	$9, donevpe		# No more VPE's
	nop

	# Disable multi-threading with TC's
	mftc0	$8, $1, 1		# C0_VPECtl
	ins	$8, $0, 15, 1		# S_VPECtlTE, W_VPECtlTE
	mttc0	$8, $1, 1		# C0_VPECtl

	beqz	TC, 1f
	nop

	# For VPE1..n
	# Clear VPA and set master VPE
	mftc0	$8, $1, 2		# C0_VPEConf0
	ins	$8, $0, 0, 1		# S_VPEConf0VPA, W_VPEConf0VPA
	or	$8, (1 << 1)		# M_VPEConf0MVP
	mttc0	$8, $1, 2		# C0_VPEConf0

	mfc0	$8, $12			# C0_Status
	mttc0	$8, $12			# C0_Status

	li	$8, 0x12345678
	mttc0	$8, $14			# C0_EPC

	mttc0	$0, $13			# C0_Cause

	mfc0	$8, $16			# C0_Config
	mttc0	$8, $16			# C0_Config

	mftc0	$8, $15, 1		# C0_EBase
	ext	$8, $8, 0, 10		# S_EBaseCPUNum, W_EBaseCPUNum
	mttgpr	$8, r23_cpu_num

	# vpe1 of each core can execute cached as it's L1 I$ has already been initialized.
    # and the L2$ has been initialized or "disabled" via CCA override.
	lui	r5_temp_addr, %hi(__reset_vector)
	addiu	r5_temp_addr, %lo(__reset_vector)
#if !defined (MTK_NAND)
	ins     r5_temp_addr, $0, 29, 1 # Convert to cached kseg0 address in case we linked to kseg1.
#endif	
	mttc0   r5_temp_addr, $2, 3	    # C0_TCRestart

	# Yes.. this is undoing all of the work done above... :)
	mftc0	$8, $2, 1		# C0_TCStatus
	ins	    $8, $0, 10, 1	# S_TCStatusIXMT, W_TCStatusIXMT
	ori	    $8, (1 << 13)	# M_TCStatusA
	mttc0	$8, $2, 1		# C0_TCStatus

	mttc0	$0, $2, 4		# C0_TCHalt

	mftc0	$8, $1, 2		# C0_VPEConf0
	ori	    $8, 1		    # M_VPEConf0VPA
	mttc0	$8, $1, 2		# C0_VPEConf0
1:

donevpe:
	addu	TC, 1
	sltu	$9, NTCS, TC
	beqz	$9, nexttc
	nop

	# Exit config mode
	mfc0	$8, $0, 1		# C0_MVPCtl
    ori     $8, 1           # set MVPControl.EVP will enable execution by vpe1!
	ins	    $8, $0, 1, 1	# S_MVPCtlVPC, W_MVPCtlVPC
	mtc0	$8, $0, 1		# C0_MVPCtl 
	ehb

#undef NTCS
#undef NVPES
#undef TC

done_init_vpe1:

    jr   r31_return_addr
    nop

/**************************************************************************************
**************************************************************************************/


/*
 * The CPS has been initialized.
 * All vpe on all cores are running main in kernel mode with their own stack and shared globals.
 * Each VPE has at least one TC. TCs beyond the number of VPEs are bound to the highest numbered VPE.
 */

#define r4_temp_data    $20
#define r5_temp_addr    $21
#define r6_temp_dest    $22
#define r7_temp_mark    $23
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
	.set    at
relocate_code:
	move	sp, a0			# set new stack pointer

	li	t0, CONFIG_SYS_MONITOR_BASE
	lui	t3, %hi(in_ram)
	addiu	t3, t3, %lo(in_ram)
	lw	t2, -12(t3)		# t2 <-- uboot_end_data
	move	t1, a2
	move	s2, a2			# s2 <-- destination address

	/*
	 * Fix $gp:
	 *
	 * New $gp = (Old $gp - CONFIG_SYS_MONITOR_BASE) + Destination Address
	 */
	move	t6, gp
	sub	gp, CONFIG_SYS_MONITOR_BASE
	add	gp, a2			# gp now adjusted
	sub	s1, gp, t6		# s1 <-- relocation offset

	/*
	 * t0 = source address
	 * t1 = target address
	 * t2 = source end address
	 */

	/*
	 * Save destination address and size for later usage in flush_cache()
	 */
	move	s0, a1			# save gd in s0
	move	a0, t1			# a0 <-- destination addr
	sub	a1, t2, t0		# a1 <-- size

1:
	lw	t3, 0(t0)
	sw	t3, 0(t1)
	addu	t0, 4
	ble	t0, t2, 1b
	 addu	t1, 4

	/* If caches were enabled, we would have to flush them here. */

	/* a0 & a1 are already set up for flush_cache(start, size) */
	//la	t9, flush_cache
	//jalr	t9
	//nop

	set_tag	0x7	
	/* Jump to where we've relocated ourselves */
	addi	t0, s2, in_ram - _start
	jr	t0
	 nop

	.word	_gp
	.word	_GLOBAL_OFFSET_TABLE_
	.word	uboot_end_data
	.word	uboot_end
	.word	num_got_entries

in_ram:
	set_tag	0x8
	/*
	 * Now we want to update GOT.
	 *
	 * GOT[0] is reserved. GOT[1] is also reserved for the dynamic object
	 * generated by GNU ld. Skip these reserved entries from relocation.
	 */
	lw	t3, -4(t0)		# t3 <-- num_got_entries
	lw	t4, -16(t0)		# t4 <-- _GLOBAL_OFFSET_TABLE_
	lw	t5, -20(t0)		# t5 <-- _gp
	sub	t4, t5			# compute offset
	add	t4, t4, gp		# t4 now holds relocated _G_O_T_
	addi	t4, t4, 8		# skipping first two entries
	li	t2, 2
1:
	lw	t1, 0(t4)
	beqz	t1, 2f
	 add	t1, s1
	sw	t1, 0(t4)
2:
	addi	t2, 1
	blt	t2, t3, 1b
	 addi	t4, 4

	/* Clear BSS */
	lw	t1, -12(t0)		# t1 <-- uboot_end_data
	lw	t2, -8(t0)		# t2 <-- uboot_end
	add	t1, s1			# adjust pointers
	add	t2, s1

	sub	t1, 4
1:
	addi	t1, 4
	bltl	t1, t2, 1b
	 sw	zero, 0(t1)

	set_tag	0x9
	move	a0, s0			# a0 <-- gd
	lui	t9, %hi(board_init_r)
	addiu	t9, t9, %lo(board_init_r)
#if 1	/* VIOSOFT: FIXME */
	lui	t8, %hi(_start)
	addiu	t8, t8, %lo(_start)
	sub	t9, t8
	add	t9, s2, t9
#endif	/* VIOSOFT */
	jr	t9
	 move	a1, s2

	.end	relocate_code

	/* Exception handlers */
romReserved:
	b	romReserved

romExcHandle:
	b	romExcHandle
