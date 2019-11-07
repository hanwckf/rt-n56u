/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

/* Define this if the system uses RELOCA.  */
#define ELF_USES_RELOCA

#include <elf.h>
/* Initialization sequence for the GOT.  */
#define INIT_GOT(GOT_BASE,MODULE)                           \
do {                                                        \
    GOT_BASE[2] = (unsigned long) _dl_linux_resolve;        \
    GOT_BASE[1] = (unsigned long) MODULE;                   \
} while(0)

/* Here we define the magic numbers that this dynamic loader should accept */
#define MAGIC1 EM_MCORE
#define MAGIC2 EM_CSKY

/* Used for error messages */
#define ELF_TARGET "csky"

struct elf_resolve;
extern unsigned long _dl_linux_resolver(struct elf_resolve * tpnt, int reloc_entry);

/* 65536 bytes alignment */
#define PAGE_ALIGN 0xfffff000          /* need modify */
#define ADDR_ALIGN 0xfff
#define OFFS_ALIGN 0x7ffff000

/* ELF_RTYPE_CLASS_PLT iff TYPE describes relocation of a PLT entry or
   TLS variable, so undefined references should not be allowed to
   define the value.
   ELF_RTYPE_CLASS_NOCOPY iff TYPE should not be allowed to resolve to one
   of the main executable's symbols, as for a COPY reloc.  */
#define elf_machine_type_class(type) \
  ((((type) == R_CKCORE_JUMP_SLOT || (type) == R_CKCORE_TLS_DTPMOD32 \
     || (type) == R_CKCORE_TLS_DTPOFF32 || (type) == R_CKCORE_TLS_TPOFF32) \
     * ELF_RTYPE_CLASS_PLT)                 \
   | (((type) == R_CKCORE_COPY) * ELF_RTYPE_CLASS_COPY))

/* Return the link-time address of _DYNAMIC.  Conveniently, this is the
   first element of the GOT.  This must be inlined in a function which
   uses global data.  */
static __inline__ Elf32_Addr elf_machine_dynamic (void) attribute_unused;
static __inline__ Elf32_Addr
elf_machine_dynamic (void)
{
    register Elf32_Addr *got __asm__ ("gb");    /* need modify */
    return *got;
}

/* this funtion will be called only when the auxvt[AT_BASE].a_un.a_val == 0
   so it normal not be called, we should define a default address of the interprrter load */
static __inline__ Elf32_Addr elf_machine_load_address (void) attribute_unused;
static __inline__ Elf32_Addr
elf_machine_load_address (void)
{
#ifdef __CSKYABIV2__
  extern Elf32_Addr internal_function __dl_start (void *) __asm__ ("_dl_start");
  Elf32_Addr got_addr = (Elf32_Addr) &__dl_start;
  Elf32_Addr pcrel_addr;
  __asm__  ("grs %0,_dl_start\n" : "=r" (pcrel_addr));
#else
  extern Elf32_Addr internal_function __start_flag (void *) __asm__ ("start_flag");
  Elf32_Addr got_addr = (Elf32_Addr) &__start_flag;
  Elf32_Addr pcrel_addr;
  __asm__  ("subi sp,8\n"           \
        "stw lr,(sp,0)\n"               \
        "bsr start_flag\n"              \
        "start_flag:"                           \
        "mov  %0, lr\n"         \
        "ldw lr,(sp,0)\n"       \
        "addi sp,8\n"           \
        : "=r" (pcrel_addr));
#endif
  return pcrel_addr - got_addr;

}

/* some relocation information are machine special */
static __inline__ void
elf_machine_relative (Elf32_Addr load_off, const Elf32_Addr rel_addr,
              Elf32_Word relative_count)
{
    Elf32_Rela *rpnt = (void *) rel_addr;
    --rpnt;
    do {
        Elf32_Addr *reloc_addr = (void *) (load_off + (++rpnt)->r_offset);
        *reloc_addr = load_off + rpnt->r_addend;
    } while (--relative_count); /* maybe need modify */
}

