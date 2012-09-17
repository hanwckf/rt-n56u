/* Copyright (C) 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

/* Somebody please explain what's going on here.  --vda */

/* This is tricky.  GCC doesn't like #include_next in the primary
   source file and even if it did, the first #include_next is this
   exact file anyway.  */
#ifndef LIBC_SIGACTION

# include <bits/libc-lock.h>

# define LIBC_SIGACTION	1

# include "sigaction.c"

# ifndef NOT_IN_libc
#  ifndef SHARED
weak_extern (__pthread_sigaction)
#  endif

int
__sigaction (sig, act, oact)
     int sig;
     const struct sigaction *act;
     struct sigaction *oact;
{
  return __libc_maybe_call2 (pthread_sigaction, (sig, act, oact),
			     __libc_sigaction (sig, act, oact));
}
# else
weak_alias (__libc_sigaction, __sigaction)
# endif
libc_hidden_weak (__sigaction)
weak_alias (__sigaction, sigaction)

#else

# include_next <sigaction.c>

#endif /* LIBC_SIGACTION */
