/*
 * Various assembly language/system dependent hacks that are required
 * so that we can minimize the amount of platform specific code.
 */

/*
 * Define this if the system uses RELOCA.
 */
#define ELF_USES_RELOCA
#include <elf.h>
/*
 * Initialization sequence for a GOT.
 */
#define INIT_GOT(GOT_BASE,MODULE)  _dl_init_got(GOT_BASE,MODULE)

/* Stuff for the PLT.  */
#define PLT_INITIAL_ENTRY_WORDS 18
#define PLT_LONGBRANCH_ENTRY_WORDS 0
#define PLT_TRAMPOLINE_ENTRY_WORDS 6
#define PLT_DOUBLE_SIZE (1<<13)
#define PLT_ENTRY_START_WORDS(entry_number) \
  (PLT_INITIAL_ENTRY_WORDS + (entry_number)*2				\
   + ((entry_number) > PLT_DOUBLE_SIZE					\
      ? ((entry_number) - PLT_DOUBLE_SIZE)*2				\
      : 0))
#define PLT_DATA_START_WORDS(num_entries) PLT_ENTRY_START_WORDS(num_entries)

/* Macros to build PowerPC opcode words.  */
#define OPCODE_ADDI(rd,ra,simm) \
  (0x38000000 | (rd) << 21 | (ra) << 16 | ((simm) & 0xffff))
#define OPCODE_ADDIS(rd,ra,simm) \
  (0x3c000000 | (rd) << 21 | (ra) << 16 | ((simm) & 0xffff))
#define OPCODE_ADD(rd,ra,rb) \
  (0x7c000214 | (rd) << 21 | (ra) << 16 | (rb) << 11)
#define OPCODE_B(target) (0x48000000 | ((target) & 0x03fffffc))
#define OPCODE_BA(target) (0x48000002 | ((target) & 0x03fffffc))
#define OPCODE_BCTR() 0x4e800420
#define OPCODE_LWZ(rd,d,ra) \
  (0x80000000 | (rd) << 21 | (ra) << 16 | ((d) & 0xffff))
#define OPCODE_LWZU(rd,d,ra) \
  (0x84000000 | (rd) << 21 | (ra) << 16 | ((d) & 0xffff))
#define OPCODE_MTCTR(rd) (0x7C0903A6 | (rd) << 21)
#define OPCODE_RLWINM(ra,rs,sh,mb,me) \
  (0x54000000 | (rs) << 21 | (ra) << 16 | (sh) << 11 | (mb) << 6 | (me) << 1)

#define OPCODE_LI(rd,simm)    OPCODE_ADDI(rd,0,simm)
#define OPCODE_ADDIS_HI(rd,ra,value) \
  OPCODE_ADDIS(rd,ra,((value) + 0x8000) >> 16)
#define OPCODE_LIS_HI(rd,value) OPCODE_ADDIS_HI(rd,0,value)
#define OPCODE_SLWI(ra,rs,sh) OPCODE_RLWINM(ra,rs,sh,0,31-sh)


#define PPC_DCBST(where) __asm__ __volatile__ ("dcbst 0,%0" : : "r"(where) : "memory")
#define PPC_SYNC __asm__ __volatile__ ("sync" : : : "memory")
#define PPC_ISYNC __asm__ __volatile__ ("sync; isync" : : : "memory")
#define PPC_ICBI(where) __asm__ __volatile__ ("icbi 0,%0" : : "r"(where) : "memory")
#define PPC_DIE __asm__ __volatile__ ("tweq 0,0")

/* Here we define the magic numbers that this dynamic loader should accept */

#define MAGIC1 EM_PPC
#undef  MAGIC2
/* Used for error messages */
#define ELF_TARGET "powerpc"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);
void _dl_init_got(unsigned long *lpnt,struct elf_resolve *tpnt);

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry, so
   PLT entries should not be allowed to define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
/* We never want to use a PLT entry as the destination of a
   reloc, when what is being relocated is a branch. This is
   partly for efficiency, but mostly so we avoid loops.  */
#define elf_machine_type_class(type) \
  ((((type) == R_PPC_JMP_SLOT				\
    || (type) == R_PPC_REL24				\
    || ((type) >= R_PPC_DTPMOD32 /* contiguous TLS */	\
	&& (type) <= R_PPC_DTPREL32)			\
    || (type) == R_PPC_ADDR24) * ELF_RTYPE_CLASS_PLT)	\
   | (((type) == R_PPC_COPY) * ELF_RTYPE_CLASS_COPY))

/* The SVR4 ABI specifies that the JMPREL relocs must be inside the
   DT_RELA table.  */
#define ELF_MACHINE_PLTREL_OVERLAP 1

/* Return the value of the GOT pointer.  */
static __always_inline Elf32_Addr * __attribute__ ((const))
ppc_got (void)
{
	Elf32_Addr *got;
#ifdef HAVE_ASM_PPC_REL16
	__asm__ ("	bcl 20,31,1f\n"
	     "1:mflr %0\n"
	     "	addis %0,%0,_GLOBAL_OFFSET_TABLE_-1b@ha\n"
	     "	addi %0,%0,_GLOBAL_OFFSET_TABLE_-1b@l\n"
	     : "=b" (got) : : "lr");
#else
	__asm__ (" bl _GLOBAL_OFFSET_TABLE_-4@local"
	     : "=l" (got));
#endif
	return got;
}

/* Return the link-time address of _DYNAMIC, stored as
   the first value in the GOT. */
static __always_inline Elf32_Addr __attribute__ ((const))
elf_machine_dynamic (void)
{
	return *ppc_got();
}

/* Return the run-time load address of the shared object.  */
static __always_inline Elf32_Addr __attribute__ ((const))
elf_machine_load_address (void)
{
  Elf32_Addr *branchaddr;
  Elf32_Addr runtime_dynamic;

  /* This is much harder than you'd expect.  Possibly I'm missing something.
     The 'obvious' way:

       Apparently, "bcl 20,31,$+4" is what should be used to load LR
       with the address of the next instruction.
       I think this is so that machines that do bl/blr pairing don't
       get confused.

     __asm__ ("bcl 20,31,0f ;"
	  "0: mflr 0 ;"
	  "lis %0,0b@ha;"
	  "addi %0,%0,0b@l;"
	  "subf %0,%0,0"
	  : "=b" (addr) : : "r0", "lr");

     doesn't work, because the linker doesn't have to (and in fact doesn't)
     update the @ha and @l references; the loader (which runs after this
     code) will do that.

     Instead, we use the following trick:

     The linker puts the _link-time_ address of _DYNAMIC at the first
     word in the GOT. We could branch to that address, if we wanted,
     by using an @local reloc; the linker works this out, so it's safe
     to use now. We can't, of course, actually branch there, because
     we'd cause an illegal instruction exception; so we need to compute
     the address ourselves. That gives us the following code: */

  /* Get address of the 'b _DYNAMIC@local'...  */
  __asm__ ("bcl 20,31,0f;"
       "b _DYNAMIC@local;"
       "0:"
       : "=l"(branchaddr));

  /* So now work out the difference between where the branch actually points,
     and the offset of that location in memory from the start of the file.  */
  runtime_dynamic = ((Elf32_Addr) branchaddr
		     + ((Elf32_Sword) (*branchaddr << 6 & 0xffffff00) >> 6));

  return runtime_dynamic - elf_machine_dynamic ();
}

static __always_inline void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
		      Elf32_Word relative_count)
{
	 Elf32_Rela * rpnt = (void *)rel_addr;
	--rpnt;
	do {     /* PowerPC handles pre increment/decrement better */
		Elf32_Addr *const reloc_addr = (void *) (load_off + (++rpnt)->r_offset);

		*reloc_addr = load_off + rpnt->r_addend;
	} while (--relative_count);
}

#define ARCH_NUM 1
#define DT_PPC_GOT_IDX	(DT_NUM + OS_NUM)

#define ARCH_DYNAMIC_INFO(dpnt,  dynamic, debug_addr) \
do { \
if (dpnt->d_tag == DT_PPC_GOT) \
     dynamic[DT_PPC_GOT_IDX] = dpnt->d_un.d_ptr; \
} while (0)
