/* Assembler macros for Nios II.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <common/sysdep.h>

#ifdef	__ASSEMBLER__

#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

#define ENTRY(name)						 \
  .globl C_SYMBOL_NAME(name);					 \
  .type C_SYMBOL_NAME(name),%function;				 \
  C_LABEL(name)

#undef  END
#define END(name)				\
  ASM_SIZE_DIRECTIVE(name)

#endif	/* __ASSEMBLER__ */
