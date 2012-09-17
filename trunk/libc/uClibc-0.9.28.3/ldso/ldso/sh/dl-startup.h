/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  */

asm(
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "_start:\n"
    "	mov	r15, r4\n"
    "	mov.l   .L_dl_start, r0\n"
    "	bsrf    r0\n"
    "	add	#4, r4\n"
    ".jmp_loc:\n"
    "	jmp	@r0\n"
    "	mov    #0, r4 	!call _start with arg == 0\n"
    ".L_dl_start:\n"
    "	.long   _dl_start-.jmp_loc\n"
    "	.size	_start,.-_start\n"
    "	.previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*)   ARGS)

/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)	\
	switch(ELF32_R_TYPE((RELP)->r_info)){			\
	case R_SH_REL32:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend		\
			    - (unsigned long)(REL);		\
		break;						\
	case R_SH_DIR32:					\
	case R_SH_GLOB_DAT:					\
	case R_SH_JMP_SLOT:					\
		*(REL)  = (SYMBOL) + (RELP)->r_addend;		\
		break;						\
	case R_SH_RELATIVE:					\
		*(REL)  = (LOAD) + (RELP)->r_addend;		\
		break;						\
	case R_SH_NONE:						\
		break;						\
	default:						\
		_dl_exit(1);					\
	}
