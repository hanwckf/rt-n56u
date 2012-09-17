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

#include <stdlib.h>

extern int __cxa_atexit (void (*func) (void *), void *arg, void *dso_handle);
libc_hidden_proto(__cxa_atexit)

/* Register a function to be called by exit or when a shared library
   is unloaded.  This routine is like __cxa_atexit, but uses the
   calling sequence required by the ARM EABI.  */
int __aeabi_atexit (void *arg, void (*func) (void *), void *d);
int __aeabi_atexit (void *arg, void (*func) (void *), void *d)
{
  return __cxa_atexit (func, arg, d);
}
