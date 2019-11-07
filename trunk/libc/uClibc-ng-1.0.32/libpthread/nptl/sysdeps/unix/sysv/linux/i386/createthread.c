/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* The "thread register" gets initialized from a segment descriptor.
   Initialize such a descriptor first.  */
#define PREPARE_CREATE \
  union user_desc_init desc;						      \
									      \
  /* Describe the thread-local storage segment.  */			      \
									      \
  /* The 'entry_number' field.  The first three bits of the segment	      \
     register value select the GDT, ignore them.  We get the index	      \
     from the value of the %gs register in the current thread.  */	      \
  desc.vals[0] = TLS_GET_GS () >> 3;					      \
  /* The 'base_addr' field.  Pointer to the TCB.  */			      \
  desc.vals[1] = (unsigned long int) pd;				      \
  /* The 'limit' field.  We use 4GB which is 0xfffff pages.  */		      \
  desc.vals[2] = 0xfffff;						      \
  /* Collapsed value of the bitfield:					      \
       .seg_32bit = 1							      \
       .contents = 0							      \
       .read_exec_only = 0						      \
       .limit_in_pages = 1						      \
       .seg_not_present = 0						      \
       .useable = 1 */							      \
  desc.vals[3] = 0x51

/* Value passed to 'clone' for initialization of the thread register.  */
#define TLS_VALUE &desc.desc


/* Get the real implementation.  */
#include <sysdeps/pthread/createthread.c>
