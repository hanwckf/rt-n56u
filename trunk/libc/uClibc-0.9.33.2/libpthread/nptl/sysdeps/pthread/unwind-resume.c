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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unwind.h>
#include <libgcc_s.h>

#define __libc_dlopen(x)        dlopen(x, (RTLD_LOCAL | RTLD_LAZY))
#define __libc_dlsym            dlsym
#define __libc_dlclose          dlclose

static void (*libgcc_s_resume) (struct _Unwind_Exception *exc);
static _Unwind_Reason_Code (*libgcc_s_personality)
  (int, _Unwind_Action, _Unwind_Exception_Class, struct _Unwind_Exception *,
   struct _Unwind_Context *);

extern
void abort(void);

static void
init (void)
{
  void *resume, *personality;
  void *handle;
  resume = personality = NULL;
  handle = dlopen (LIBGCC_S_SO, (RTLD_LOCAL | RTLD_LAZY));

  if (handle == NULL
      || (resume = dlsym (handle, "_Unwind_Resume")) == NULL
      || (personality = dlsym (handle, "__gcc_personality_v0")) == NULL)
  {
    printf (LIBGCC_S_SO " must be installed for pthread_cancel to work\n");
    abort();
  }

  libgcc_s_resume = resume;
  libgcc_s_personality = personality;
}

void
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__builtin_expect (libgcc_s_resume == NULL, 0))
    init ();
  libgcc_s_resume (exc);
}

_Unwind_Reason_Code
__gcc_personality_v0 (int version, _Unwind_Action actions,
		      _Unwind_Exception_Class exception_class,
                      struct _Unwind_Exception *ue_header,
                      struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_personality == NULL, 0))
    init ();
  return libgcc_s_personality (version, actions, exception_class,
			       ue_header, context);
}
