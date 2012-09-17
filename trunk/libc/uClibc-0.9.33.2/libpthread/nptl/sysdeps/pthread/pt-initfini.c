/* Special .init and .fini section support.  Linuxthread version.
   Copyright (C) 1995,1996,1997,2000,2001,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file.  (The Library General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   The GNU C Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file is compiled into assembly code which is then munged by a sed
   script into two files: crti.s and crtn.s.

   * crti.s puts a function prologue at the beginning of the
   .init and .fini sections and defines global symbols for
   those addresses, so they can be called as functions.

   * crtn.s puts the corresponding function epilogues
   in the .init and .fini sections. */

#include <stdlib.h>

/* We use embedded asm for .section unconditionally, as this makes it
   easier to insert the necessary directives into crtn.S. */
#define SECTION(x) __asm__ (".section " x )

/* Embed an #include to pull in the alignment and .end directives. */
__asm__ ("\n#include \"defs.h\"");
__asm__ ("\n#if defined __i686 && defined __ASSEMBLER__");
__asm__ ("\n#undef __i686");
__asm__ ("\n#define __i686 __i686");
__asm__ ("\n#endif");

/* The initial common code ends here. */
__asm__ ("\n/*@HEADER_ENDS*/");

/* To determine whether we need .end and .align: */
__asm__ ("\n/*@TESTS_BEGIN*/");
extern void dummy (void (*foo) (void));
void
dummy (void (*foo) (void))
{
  if (foo)
    (*foo) ();
}
__asm__ ("\n/*@TESTS_END*/");

/* The beginning of _init:  */
__asm__ ("\n/*@_init_PROLOG_BEGINS*/");

static void
call_initialize_minimal (void)
{
  extern void __pthread_initialize_minimal_internal (void)
    __attribute ((visibility ("hidden")));

  __pthread_initialize_minimal_internal ();
}

SECTION (".init");
extern void __attribute__ ((section (".init"))) _init (void);
void
_init (void)
{
  /* The very first thing we must do is to set up the registers.  */
  call_initialize_minimal ();

  __asm__ ("ALIGN");
  __asm__("END_INIT");
  /* Now the epilog. */
  __asm__ ("\n/*@_init_PROLOG_ENDS*/");
  __asm__ ("\n/*@_init_EPILOG_BEGINS*/");
  SECTION(".init");
}
__asm__ ("END_INIT");

/* End of the _init epilog, beginning of the _fini prolog. */
__asm__ ("\n/*@_init_EPILOG_ENDS*/");
__asm__ ("\n/*@_fini_PROLOG_BEGINS*/");

SECTION (".fini");
extern void __attribute__ ((section (".fini"))) _fini (void);
void
_fini (void)
{

  /* End of the _fini prolog. */
  __asm__ ("ALIGN");
  __asm__ ("END_FINI");
  __asm__ ("\n/*@_fini_PROLOG_ENDS*/");

  {
    /* Let GCC know that _fini is not a leaf function by having a dummy
       function call here.  We arrange for this call to be omitted from
       either crt file.  */
    extern void i_am_not_a_leaf (void);
    i_am_not_a_leaf ();
  }

  /* Beginning of the _fini epilog. */
  __asm__ ("\n/*@_fini_EPILOG_BEGINS*/");
  SECTION (".fini");
}
__asm__ ("END_FINI");

/* End of the _fini epilog.  Any further generated assembly (e.g. .ident)
   is shared between both crt files. */
__asm__ ("\n/*@_fini_EPILOG_ENDS*/");
__asm__ ("\n/*@TRAILER_BEGINS*/");

/* End of file. */
