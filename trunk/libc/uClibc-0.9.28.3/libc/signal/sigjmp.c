/* Copyright (C) 1992, 1994, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <setjmp.h>
#include <signal.h>

/* This function is called by the `sigsetjmp' macro
   before doing a `__setjmp' on ENV[0].__jmpbuf.
   Always return zero.  */

int __sigjmp_save (sigjmp_buf env, int savemask)
{
    env[0].__mask_was_saved = (savemask && 
	    sigprocmask (SIG_BLOCK, (sigset_t *) NULL, &env[0].__saved_mask) == 0);

    return 0;
}
