/* Copyright (C) 2003 Red Hat, Inc.
 * Contributed by Alexandre Oliva <aoliva@redhat.com>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Any assembly language/system dependent hacks needed to setup
 * boot1.c so it will work as expected and cope with whatever platform
 * specific wierdness is needed for this architecture.

 * We override the default _dl_boot function, and replace it with a
 * bit of asm.  Then call the real _dl_boot function, which is now
 * named _dl_boot2.  */

/* At program start-up, gr16 contains a pointer to a
   elf32_fdpic_loadmap that describes how the executable was loaded
   into memory.  gr17 contains a pointer to the interpreter (our!)
   loadmap, if there is an interpreter, or 0 if we're being run as an
   executable.  gr18 holds a pointer to the interpreter's dynamic
   section, if there is an interpreter, or to the executable's dynamic
   section, otherwise.  If the executable is not dynamic, gr18 is 0.

   We rely on the fact that the linker adds a pointer to the
   _GLOBAL_OFFSET_TABLE_ as the last ROFIXUP entry, and that
   __self_reloc returns the relocated pointer to us, so that we can
   use this value to initialize the PIC register.  */

__asm__("" \
"	.text\n"			\
"	.global	_start\n"		\
"	.type	_start,@function\n"	\
"	.hidden	_start\n"		\
"_start:\n"				\
"	call	.Lcall\n"		\
".Lcall:\n"				\
"	movsg	lr, gr4\n"		\
"	sethi.p	#gprelhi(.Lcall), gr5\n"\
"	setlo	#gprello(.Lcall), gr5\n"\
"	mov.p	gr17, gr8\n"		\
"	cmp	gr17, gr0, icc0\n"	\
"	sub.p	gr4, gr5, gr4\n"	\
"	ckeq	icc0, cc4\n"		\
"	cmov.p	gr16, gr8, cc4, 1\n"	\
"	sethi	#gprelhi(__ROFIXUP_LIST__), gr9\n"	\
"	sethi.p	#gprelhi(__ROFIXUP_END__), gr10\n"	\
"	setlo	#gprello(__ROFIXUP_LIST__), gr9\n"	\
"	setlo.p	#gprello(__ROFIXUP_END__), gr10\n"	\
"	add	gr9, gr4, gr9\n"	\
"	add.p	gr10, gr4, gr10\n"	\
"	call	__self_reloc\n"		\
"	mov.p	gr8, gr15\n"		\
"	mov	gr16, gr9\n"		\
"	mov.p	gr17, gr10\n"		\
"	mov	gr18, gr11\n"		\
"	addi.p	sp, #4, gr13\n"		\
"	addi	sp, #-8, sp\n"		\
"	mov.p	sp, gr12\n"		\
"	call	_dl_start\n"		\
"	ldd.p	@(sp, gr0), gr14\n"	\
"	addi	sp, #8, sp\n"		\
"	movgs	gr0, lr\n"		\
"	jmpl	@(gr14, gr0)\n"		\
"	.size	_start,.-_start\n"	\
);

#undef DL_START
#define DL_START(X)   \
static void  __attribute__ ((used)) \
_dl_start (Elf32_Addr dl_boot_got_pointer, \
	  struct elf32_fdpic_loadmap *dl_boot_progmap, \
	  struct elf32_fdpic_loadmap *dl_boot_ldsomap, \
	  Elf32_Dyn *dl_boot_ldso_dyn_pointer, \
	  struct funcdesc_value *dl_main_funcdesc, \
	  X)

/*
 * Get a pointer to the argv array.  On many platforms this can be just
 * the address of the first argument, on other platforms we need to
 * do something a little more subtle here.
 */
#define GET_ARGV(ARGVP, ARGS) ARGVP = ((unsigned long*) ARGS)

/*
 * Here is a macro to perform a relocation.  This is only used when
 * bootstrapping the dynamic loader.  RELP is the relocation that we
 * are performing, REL is the pointer to the address we are relocating.
 * SYMBOL is the symbol involved in the relocation, and LOAD is the
 * load address.
 */
#define PERFORM_BOOTSTRAP_RELOC(RELP,REL,SYMBOL,LOAD,SYMTAB) \
	switch(ELF_R_TYPE((RELP)->r_info)){				\
	case R_FRV_32:							\
	  *(REL) += (SYMBOL);						\
	  break;							\
	case R_FRV_FUNCDESC_VALUE:					\
	  {								\
	    struct funcdesc_value fv = {				\
	      (void*)((SYMBOL) + *(REL)),				\
	      (LOAD).got_value						\
	    };								\
	    *(struct funcdesc_value volatile *)(REL) = fv;		\
	    break;							\
	  }								\
	default:							\
	  _dl_exit(1);							\
	}

/*
 * Transfer control to the user's application, once the dynamic loader
 * is done.  We return the address of the function's entry point to
 * _dl_boot, see boot1_arch.h.
 */
#define START()	do {							\
  struct elf_resolve *exec_mod = _dl_loaded_modules;			\
  dl_main_funcdesc->entry_point = _dl_elf_main;				\
  while (exec_mod->libtype != elf_executable)				\
    exec_mod = exec_mod->next;						\
  dl_main_funcdesc->got_value = exec_mod->loadaddr.got_value;		\
  return;								\
} while (0)
