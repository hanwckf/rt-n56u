#include <common/sysdep.h>

#ifdef __ASSEMBLER__

/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
# define ENTRY(name)							      \
  .globl C_SYMBOL_NAME(name);						      \
  .type C_SYMBOL_NAME(name),@function;					      \
  .p2align 2;								      \
  C_LABEL(name)								      \
  cfi_startproc;							      \

# undef END
# define END(name)							      \
  cfi_endproc;								      \
  .size name,.-name

/* Load the address of the GOT into register R.  */
# define LOAD_GOT(R) \
  lea _GLOBAL_OFFSET_TABLE_@GOTPC (%pc), R

#endif
