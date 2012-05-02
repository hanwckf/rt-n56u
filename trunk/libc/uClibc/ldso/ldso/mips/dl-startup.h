/* Any assmbly language/system dependent hacks needed to setup boot1.c so it
 * will work as expected and cope with whatever platform specific wierdness is
 * needed for this architecture.
 * Copyright (C) 2005 by Joakim Tjernlund
 * Copyright (C) 2005 by Erik Andersen
 */


asm(""
    "	.text\n"
    "	.globl	_start\n"
    "	.ent	_start\n"
    "	.type	_start,@function\n"
    "_start:\n"
    "	.set noreorder\n"
    "	move	$25, $31\n"
    "	bal	0f\n"
    "	nop\n"
    "0:\n"
    "	.cpload	$31\n"
    "	move	$31, $25\n"
    "	.set reorder\n"
    "	la	$4, _DYNAMIC\n"
    "	sw	$4, -0x7ff0($28)\n"
    "	move	$4, $29\n"
    "	subu	$29, 16\n"
    "	la	$8, .coff\n"
    "	bltzal	$8, .coff\n"
    ".coff:\n"
    "	subu	$8, $31, $8\n"
    "	la	$25, _dl_start\n"
    "	addu	$25, $8\n"
    "	jalr	$25\n"
    "	addiu	$29, 16\n"
    "	move	$16, $28\n"
    "	move	$17, $2\n"
    "	lw	$2, _dl_skip_args\n"
    "	beq	$2, $0, 1f\n"
    "	lw	$4, 0($29)\n"
    "	subu	$4, $2\n"
    "	sll	$2, 2\n"
    "	addu	$29, $2\n"
    "	sw	$4, 0($29)\n"
    "1:\n"
    "	lw	$5, 0($29)\n"
    "	la	$6, 4 ($29)\n"
    "	sll	$7, $5, 2\n"
    "	addu	$7, $7, $6\n"
    "	addu	$7, $7, 4\n"
    "	and	$2, $29, -2 * 4\n"
    "	sw	$29, -4($2)\n"
    "	subu	$29, $2, 32\n"
    "	.cprestore 16\n"
    "	lw	$29, 28($29)\n"
    "	la	$2, _dl_fini\n"
    "	move	$25, $17\n"
    "	jr	$25\n"
    ".end	_start\n"
    ".size	_start, . -_start\n"
    "\n\n"
    "\n\n"
    ".previous\n"
);

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address if the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = (((unsigned long *) ARGS)+1)


/* We can't call functions earlier in the dl startup process */
#define NO_FUNCS_BEFORE_BOOTSTRAP


/*
 * Here is a macro to perform the GOT relocation. This is only
 * used when bootstrapping the dynamic loader.
 */
#define PERFORM_BOOTSTRAP_GOT(tpnt)						\
do {										\
	Elf32_Sym *sym;								\
	Elf32_Addr i;								\
	register ElfW(Addr) gp __asm__ ("$28");					\
	Elf32_Addr *mipsgot = elf_mips_got_from_gpreg (gp);			\
										\
	/* Add load address displacement to all local GOT entries */		\
	i = 2;									\
	while (i < tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX])			\
		mipsgot[i++] += tpnt->loadaddr;					\
										\
	/* Handle global GOT entries */						\
	mipsgot += tpnt->dynamic_info[DT_MIPS_LOCAL_GOTNO_IDX];			\
	sym = (Elf32_Sym *) tpnt->dynamic_info[DT_SYMTAB] +			\
		 	tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];			\
	i = tpnt->dynamic_info[DT_MIPS_SYMTABNO_IDX] - tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX];\
										\
	while (i--) {								\
		if (sym->st_shndx == SHN_UNDEF ||				\
			sym->st_shndx == SHN_COMMON)				\
			*mipsgot = tpnt->loadaddr + sym->st_value;		\
		else if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC &&		\
			*mipsgot != sym->st_value)				\
			*mipsgot += tpnt->loadaddr;				\
		else if (ELF32_ST_TYPE(sym->st_info) == STT_SECTION) {		\
			if (sym->st_other == 0)					\
				*mipsgot += tpnt->loadaddr;			\
		}								\
		else								\
			*mipsgot = tpnt->loadaddr + sym->st_value;		\
										\
		mipsgot++;							\
		sym++;								\
	}									\
} while (0)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB)			\
	switch(ELF32_R_TYPE((RELP)->r_info)) {					\
	case R_MIPS_REL32:							\
		if (SYMTAB) {							\
			if (symtab_index<tpnt->dynamic_info[DT_MIPS_GOTSYM_IDX])\
				*REL += SYMBOL;					\
		}								\
		else {								\
			*REL += LOAD;						\
		}								\
		break;								\
	case R_MIPS_NONE:							\
		break;								\
	default:								\
		SEND_STDERR("Aiieeee!");					\
		_dl_exit(1);							\
	}
