/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>


/* Clear memory.  Can't alias to bzero because it's not defined in the
   same translation unit.  */
void __aeabi_memclr (void *dest, size_t n);
void __aeabi_memclr (void *dest, size_t n)
{
  memset (dest, 0, n);
}

/* Versions of the above which may assume memory alignment.  */
strong_alias (__aeabi_memclr, __aeabi_memclr4)
strong_alias (__aeabi_memclr, __aeabi_memclr8)
