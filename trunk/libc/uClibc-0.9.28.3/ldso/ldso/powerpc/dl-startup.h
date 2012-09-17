/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 * Copyright (C) 2005 by Joakim Tjernlund
 */

asm(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "_start:\n"
    "	mr	3,1\n" /* Pass SP to _dl_start in r3 */
    "	li	0,0\n"
    "	stwu	1,-16(1)\n" /* Make room on stack for _dl_start to store LR */
    "	stw	0,0(1)\n" /* Clear Stack frame */
    "	bl	_dl_start@local\n" /* Perform relocation */
    /*  Save the address of the apps entry point in CTR register */
    "	mtctr	3\n" /* application entry point */
#ifdef HAVE_ASM_PPC_REL16
    "	bcl	20,31,1f\n"
    "1:	mflr	31\n"
    "	addis	31,31,_GLOBAL_OFFSET_TABLE_-1b@ha\n"
    "	addi	31,31,_GLOBAL_OFFSET_TABLE_-1b@l\n"
#else
    "	bl	_GLOBAL_OFFSET_TABLE_-4@local\n" /*  Put our GOT pointer in r31, */
    "	mflr	31\n"
#endif
    "	addi	1,1,16\n" /* Restore SP */
    "	lwz	7,_dl_skip_args@got(31)\n" /* load EA of _dl_skip_args */
    "	lwz	7,0(7)\n"	/* Load word from _dl_skip_args */
    "	lwz	8,0(1)\n"	/* Load argc from stack */
    "	subf	8,7,8\n"	/* Subtract _dl_skip_args from argc. */
    "	slwi	7,7,2\n"	/* Multiply by 4 */
    "	stwux	8,1,7\n"	/* Adjust the stack pointer to skip _dl_skip_args words and store adjusted argc on stack. */
#if 0
    /* Try beeing SVR4 ABI compliant?, even though it is not needed for uClibc on Linux */
    /* argc */
    "	lwz	3,0(1)\n"
    /* find argv one word offset from the stack pointer */
    "	addi	4,1,4\n"
    /* find environment pointer (argv+argc+1) */
    "	lwz	5,0(1)\n"
    "	addi	5,5,1\n"
    "	rlwinm	5,5,2,0,29\n"
    "	add	5,5,4\n"
    /* pass the auxilary vector in r6. This is passed to us just after _envp.  */
    "2:	lwzu	0,4(6)\n"
    "	cmpwi	0,0\n"
    "	bne	2b\n"
    "	addi	6,6,4\n"
#endif
    /* Pass a termination function pointer (in this case _dl_fini) in r3. */
    /* Paulus promized he would keep r3 zero in the exec ABI. */
    "	lwz	3,_dl_fini@got(31)\n"
    "	mr	7,3\n"		/* Pass _dl_fini in r7 to maintain compat */
    "	bctr\n" /* Jump to entry point */
    "	.size	_start,.-_start\n"
    "	.previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*) ARGS)+1)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
	{int type=ELF32_R_TYPE((RELP)->r_info);		\
	 Elf32_Addr finaladdr=(SYMBOL)+(RELP)->r_addend;\
	if (type==R_PPC_RELATIVE) {			\
		*REL=(Elf32_Word)(LOAD)+(RELP)->r_addend;\
	} else if (type==R_PPC_ADDR32 || type==R_PPC_GLOB_DAT) {\
		*REL=finaladdr;				\
	} else if (type==R_PPC_JMP_SLOT) {		\
		Elf32_Sword delta=finaladdr-(Elf32_Word)(REL);\
		*REL=OPCODE_B(delta);			\
		PPC_DCBST(REL); PPC_SYNC; PPC_ICBI(REL);\
	} else {					\
		_dl_exit(100+ELF32_R_TYPE((RELP)->r_info));\
	}						\
	}
