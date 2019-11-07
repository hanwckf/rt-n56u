/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>
#include <libgcc_s.h>
#include <unwind-resume.h>

#define __libc_dlopen(x)        dlopen(x, (RTLD_LOCAL | RTLD_LAZY))
#define __libc_dlsym            dlsym
#define __libc_dlclose          dlclose

void (*__libgcc_s_resume) (struct _Unwind_Exception *exc)
  attribute_hidden __attribute__ ((noreturn));

static _Unwind_Reason_Code (*libgcc_s_personality) PERSONALITY_PROTO;

extern
void abort(void);

void attribute_hidden __attribute__ ((cold))
__libgcc_s_init(void)
{
  void *resume, *personality;
  void *handle;

  handle = __libc_dlopen (LIBGCC_S_SO);

  if (handle == NULL
      || (resume = __libc_dlsym (handle, "_Unwind_Resume")) == NULL
      || (personality = __libc_dlsym (handle, "__gcc_personality_v0")) == NULL)
  {
    fprintf (stderr,
	     LIBGCC_S_SO " must be installed for pthread_cancel to work\n");
    abort();
  }

  __libgcc_s_resume = resume;
  libgcc_s_personality = personality;
}

#if !HAVE_ARCH_UNWIND_RESUME
void attribute_hidden
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__builtin_expect (libgcc_s_resume == NULL, 0))
    __libgcc_s_init ();
  __libgcc_s_resume (exc);
}
#endif

_Unwind_Reason_Code attribute_hidden
__gcc_personality_v0 PERSONALITY_PROTO
{
  if (__builtin_expect (libgcc_s_personality == NULL, 0))
    __libgcc_s_init ();
  return libgcc_s_personality PERSONALITY_ARGS;
}
