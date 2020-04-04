/* CRIS can never use Elf32_Rel relocations. */
#define ELF_USES_RELOCA

#include <elf.h>

/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)				\
{								\
	GOT_BASE[1] = (unsigned long) MODULE; 			\
	GOT_BASE[2] = (unsigned long) _dl_linux_resolve; 	\
}

/* Defined some magic numbers that this ld.so should accept. */
#define MAGIC1 EM_CRIS
#undef MAGIC2
#define ELF_TARGET "CRIS"

/* Need bootstrap relocations */
#define ARCH_NEEDS_BOOTSTRAP_RELOCS

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve *tpnt, int reloc_entry);

/* The union of reloc-type-classes where the reloc TYPE is a member.

   TYPE is in the class ELF_RTYPE_CLASS_PLT if it can describe a
   relocation for a PLT entry, that is, for which a PLT entry should not
   be allowed to define the value.  The GNU linker for CRIS can merge a
   .got.plt entry (R_CRIS_JUMP_SLOT) with a .got entry (R_CRIS_GLOB_DAT),
   so we need to match both these reloc types.

   TYPE is in the class ELF_RTYPE_CLASS_NOCOPY if it should not be allowed
   to resolve to one of the main executable's symbols, as for a COPY
   reloc.  */
#define elf_machine_type_class(type)				\
  ((((((type) == R_CRIS_JUMP_SLOT))				\
     || ((type) == R_CRIS_GLOB_DAT)) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_CRIS_COPY) * ELF_RTYPE_CLASS_COPY))

static __always_inline Elf32_Addr
elf_machine_dynamic(void)
{
	/* Don't just set this to an asm variable "r0" since that's not logical
	   (like, the variable is uninitialized and the register is fixed) and
	   may make GCC trip over itself doing register allocation.  Yes, I'm
	   paranoid.  Why do you ask?  */
	Elf32_Addr *got;

	__asm__ ("move.d $r0,%0" : "=rm" (got));
	return *got;
}

/* Return the run-time load address of the shared object.  We do it like
   m68k and i386, by taking an arbitrary local symbol, forcing a GOT entry
   for it, and peeking into the GOT table, which is set to the link-time
   file-relative symbol value (regardless of whether the target is REL or
   RELA).  We subtract this link-time file-relative value from the "local"
   value we calculate from GOT position and GOT offset.  FIXME: Perhaps
   there's some other symbol we could use, that we don't *have* to force a
   GOT entry for.  */

static __always_inline Elf32_Addr
elf_machine_load_address(void)
{
	Elf32_Addr gotaddr_diff;

#ifdef __arch_v32
	extern char ___CRISv32_dummy[] __asm__ ("_dl_start");

	__asm__ ("addo.w _dl_start:GOT16,$r0,$acr\n\t"
	         "lapc _dl_start,%0\n\t"
	         "sub.d [$acr],%0"
	         /* For v32, we need to force GCC to have R0 loaded with
	            _GLOBAL_OFFSET_TABLE_ at this point, which might not
	            otherwise have happened in the caller.  (For v10, it's
	            loaded for non-global variables too, so we don't need
	            anything special there.)  We accomplish this by faking the
	            address of a global variable (as seen by GCC) as input to
	            the asm; that address calculation goes through the GOT.
	            Use of this function happens before we've filled in the
	            GOT, so the address itself will not be correctly
	            calculated, therefore we don't use any symbol whose
	            address may be re-used later on.  Let's just reuse the
	            _dl_start symbol, faking it as a global by renaming it as
	            another variable through an asm.  */
	         : "=r" (gotaddr_diff)
	         : "g" (___CRISv32_dummy)
	         : "acr");
#else
	__asm__ ("sub.d [$r0+_dl_start:GOT16],$r0,%0\n\t"
	         "add.d _dl_start:GOTOFF,%0" : "=r" (gotaddr_diff));
#endif
	return gotaddr_diff;
}

static __always_inline void
elf_machine_relative(Elf32_Addr load_off, const Elf32_Addr rel_addr,
                     Elf32_Word relative_count)
{
	Elf32_Rela *rpnt = (void *)rel_addr;

	--rpnt;
	do {
		Elf32_Addr *const reloc_addr =
			(void *)(load_off + (++rpnt)->r_offset);

		*reloc_addr =  load_off + rpnt->r_addend;
	} while (--relative_count);
}
