/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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

#include <fpu_control.h>
#include <stdio.h>

int
main (void)
{
#ifdef _FPU_GETCW
/* Some architectures don't have _FPU_GETCW (e.g. Linux/Alpha).  */
  fpu_control_t cw;

  _FPU_GETCW (cw);

  cw &= ~_FPU_RESERVED;

  if (cw != (_FPU_DEFAULT & ~_FPU_RESERVED))
    printf ("control word is 0x%lx but should be 0x%lx.\n",
	    (long int) cw, (long int) (_FPU_DEFAULT & ~_FPU_RESERVED));

  return cw != (_FPU_DEFAULT & ~_FPU_RESERVED);

#else
  return 0;
#endif
}
