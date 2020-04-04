/* Any assembly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.  */

__asm__(
    "	.text\n"
    "	.globl	_start\n"
    "	.type	_start,@function\n"
    "	.hidden	_start\n"
    "_start:\n"
    "	mov	r15, r4\n"
    "	mov.l   .L_dl_start, r0\n"
    "	bsrf    r0\n"
    "	add	#4, r4\n"
    ".jmp_loc:\n"
    "	mov     r0, r8        ! Save the user entry point address in r8\n"
    "	mov.l   .L_got, r12   ! Load the GOT on r12\n"
    "	mova    .L_got, r0\n"
    "	add     r0, r12\n"
    "	mov.l .L_dl_skip_args,r0\n"
    "	mov.l @(r0,r12),r0\n"
    "	mov.l @r0,r0\n"
    "	mov.l @r15,r5         ! Get the original argument count\n"
    "	sub r0,r5             ! Subtract _dl_skip_args from it\n"
    "	shll2 r0\n"
    "	add r0,r15 ! Adjust the stack pointer to skip _dl_skip_args words\n"
    "	mov.l r5,@r15         ! Store back the modified argument count\n"
    "	mov.l   .L_dl_fini, r0\n"
    "	mov.l   @(r0,r12), r4 ! Pass the finalizer in r4\n"
    "	jmp     @r8\n"
    "	nop\n"
    ".L_dl_start:\n"
    "	.long   _dl_start-.jmp_loc\n"
    ".L_dl_skip_args:\n"
    "	.long   _dl_skip_args@GOT\n"
    ".L_dl_fini:\n"
    "	.long	_dl_fini@GOT\n"
    ".L_got:\n"
    "	.long _GLOBAL_OFFSET_TABLE_\n"
    "	.size	_start,.-_start\n"
    "	.previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
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
	switch(ELF_R_TYPE((RELP)->r_info)){			\
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
