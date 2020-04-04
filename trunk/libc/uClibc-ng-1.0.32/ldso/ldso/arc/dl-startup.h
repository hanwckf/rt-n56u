/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/*
 * vineetg: Refactoring/cleanup of loader entry point
 *  Removed 6 useless insns
 * Joern Improved it even further:
 *  -better insn scheduling
 *  -no need for conditional code for _dl_skip_args
 *  -use of assembler .&2 expressions vs. @gotpc refs (avoids need for GP)
 *
 * What this code does:
 *  -ldso starts execution here when kernel returns from execve()
 *  -calls into generic ldso entry point _dl_start( )
 *  -optionally adjusts argc for executable if exec passed as cmd
 *  -calls into app main with address of finaliser
 */
__asm__(
    ".section .text                                             \n"
    ".align 4                                                  	\n"
    ".global _start                                             \n"
    ".hidden _start                                             \n"
    ".type  _start,@function                                    \n"

    "_start:                                                    \n"
    "   ; ldso entry point, returns app entry point             \n"
    "   bl.d    _dl_start                                       \n"
    "   mov_s   r0, sp          ; pass ptr to aux vector tbl    \n"

    "   ; If ldso ran as cmd with executable file nm as arg     \n"
    "   ; skip the extra args calc by dl_start()                \n"
    "   ld_s    r1, [sp]       ; orig argc from aux-vec Tbl     \n"

    "   ld      r12, [pcl, _dl_skip_args@pcl]                   \n"
    "   add     r2, pcl, _dl_fini@pcl       ; finalizer         \n"

    "   add2    sp, sp, r12    ; discard argv entries from stack\n"
    "   sub_s   r1, r1, r12    ; adjusted argc, on stack        \n"
    "   st_s    r1, [sp]                                        \n"

    "   j_s.d   [r0]           ; app entry point                \n"
    "   mov_s   r0, r2         ; ptr to finalizer _dl_fini      \n"

    ".size  _start,.-_start                                     \n"
    ".previous                                                  \n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*) ARGS + 1)

/*
 * Dynamic loader bootstrapping:
 * The only relocations that should be found are either R_ARC_RELATIVE for
 * data relocations (.got, etc) or R_ARC_JMP_SLOT for code relocations
 * (.plt).  It is safe to assume that all of these relocations are word
 * aligned.
 * @RELP is the reloc entry being processed
 * @REL is the pointer to the address we are relocating.
 * @SYMBOL is the symbol involved in the relocation
 * @LOAD is the load address.
 */

#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)		\
do {									\
	int type = ELF32_R_TYPE((RELP)->r_info);			\
	if (likely(type == R_ARC_RELATIVE))				\
		*REL += (unsigned long) LOAD;				\
	else if (type == R_ARC_JMP_SLOT)                                \
		*REL = SYMBOL;						\
	else if (type != R_ARC_NONE)					\
		_dl_exit(1);						\
}while(0)

/*
 * This will go away once we have DT_RELACOUNT
 */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

/* we dont need to spit out argc, argv etc for debugging */
#define NO_EARLY_SEND_STDERR    1
