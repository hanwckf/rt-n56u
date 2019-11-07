/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS


__asm__("\
	.text\n\
	.globl 	_start\n\
	.globl  _dl_start\n\
	.globl 	_dl_start_user\n\
    .type	_start,#function\n\
    .type	_dl_start,#function\n\
	.type 	_dl_start_user,#function\n\
	.align	4\n\
	.pic\n\
_start:\n\
	! we are PIC code, so get global offset table\n\
	mfusr	$r15, 	$PC \n\
	sethi	$gp, 	HI20(_GLOBAL_OFFSET_TABLE_ + 4)\n\
	ori		$gp, 	$gp, LO12(_GLOBAL_OFFSET_TABLE_ + 8)\n\
	add		$gp, 	$r15, $gp\n\
\n\
	! at start time, all the args are on the stack\n\
	addi	$r0,	$sp,	0\n\
    ! adjust stack\n\
    !addi    $sp,    $sp,    -24\n\
	bal	_dl_start@PLT\n\
	! save user entry point in r6\n\
	addi	$r6,	$r0,	0\n\
    ! adjust sp and reload registers\n\
    !addi    $sp,    $sp,    24\n\
\n\
_dl_start_user:\n\
	! See if we were run as a command with the executable file\n\
	! name as an extra leading argument.\n\
	! skip these arguments\n\
	l.w		$r2,	_dl_skip_args@GOTOFF	! args to skip\n\
	lwi		$r0,	[$sp+0]					! original argc\n\
	slli	$r1,	$r2,	2				! offset for new sp\n\
	add		$sp,	$sp,	$r1				! adjust sp to skip args\n\
	sub		$r0,	$r0,	$r2				! set new argc\n\
	swi		$r0,	[$sp+0]					! save new argc\n\
\n\
	! load address of _dl_fini finalizer function\n\
	la		$r5, _dl_fini@GOTOFF\n\
	! jump to the user_s entry point\n\
	jr	$r6\n\
	.size   _dl_start_user, . - _dl_start_user\n\
	.previous\n\
");

#define COPY_UNALIGNED_WORD(swp, twp) \
{ \
        __typeof (swp) __tmp = __builtin_nds32_unaligned_load_w ((unsigned int*)&swp); \
        __builtin_nds32_unaligned_store_w ((unsigned int *)twp, __tmp); \
}

/* Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.  */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long*)ARGS)+1)

/* Handle relocation of the symbols in the dynamic loader. */
static __always_inline
void PERFORM_BOOTSTRAP_RELOC(ELF_RELOC *rpnt, unsigned long *reloc_addr,
	unsigned long symbol_addr, unsigned long load_addr, Elf32_Sym *symtab)
{
	Elf32_Addr value;
	switch (ELF32_R_TYPE(rpnt->r_info)) {
		case R_NDS32_NONE:
			break;
		case R_NDS32_32:
		case R_NDS32_GLOB_DAT:
		case R_NDS32_JMP_SLOT:
			*reloc_addr = symbol_addr + rpnt->r_addend;
			break;
		case R_NDS32_32_RELA:
			value = symbol_addr + rpnt->r_addend;
			COPY_UNALIGNED_WORD (value, reloc_addr);
			break;
#undef COPY_UNALIGNED_WORD
		case R_NDS32_RELATIVE:
			*reloc_addr = load_addr + rpnt->r_addend;
			break;
		default:
			SEND_STDERR("Unsupported relocation type\n");
			_dl_exit(1);
	}
}
