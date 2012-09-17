/* Copyright (C) 2002, 2003, 2005, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <unistd.h>
#include <sysdep.h>


static const char banner[] =
#include "banner.h"
"Copyright (C) 2006 Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.\n"
#ifdef HAVE_FORCED_UNWIND
"Forced unwind support included.\n"
#endif
;


extern void __nptl_main (void) __attribute__ ((noreturn));
void
__nptl_main (void)
{
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (write, err, 3, STDOUT_FILENO, (const char *) banner,
		    sizeof banner - 1);

  _exit (0);
}
