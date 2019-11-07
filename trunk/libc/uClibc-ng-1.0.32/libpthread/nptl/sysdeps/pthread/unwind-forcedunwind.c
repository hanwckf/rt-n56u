/* Copyright (C) 2003, 2005, 2006, 2009 Free Software Foundation, Inc.
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
#include <unwind.h>
#include <pthreadP.h>
#include <sysdep.h>
#include <libgcc_s.h>
#include <unwind-resume.h>

#define __libc_dlopen(x)        dlopen(x, (RTLD_LOCAL | RTLD_LAZY))
#define __libc_dlsym            dlsym
#define __libc_dlclose		dlclose

static void *libgcc_s_handle;
void (*__libgcc_s_resume) (struct _Unwind_Exception *exc)
  attribute_hidden __attribute__ ((noreturn));
static _Unwind_Reason_Code (*libgcc_s_personality) PERSONALITY_PROTO;
static _Unwind_Reason_Code (*libgcc_s_forcedunwind)
  (struct _Unwind_Exception *, _Unwind_Stop_Fn, void *);
static _Unwind_Word (*libgcc_s_getcfa) (struct _Unwind_Context *);

void
__attribute_noinline__
pthread_cancel_init (void)
{
  void *resume;
  void *personality;
  void *forcedunwind;
  void *getcfa;
  void *handle;

  if (__builtin_expect (libgcc_s_handle != NULL, 1))
    {
      /* Force gcc to reload all values.  */
      __asm__ __volatile__ ("" ::: "memory");
      return;
    }

  handle = __libc_dlopen (LIBGCC_S_SO);

  if (handle == NULL
      || (resume = __libc_dlsym (handle, "_Unwind_Resume")) == NULL
      || (personality = __libc_dlsym (handle, "__gcc_personality_v0")) == NULL
      || (forcedunwind = __libc_dlsym (handle, "_Unwind_ForcedUnwind"))
	 == NULL
      || (getcfa = __libc_dlsym (handle, "_Unwind_GetCFA")) == NULL
#ifdef ARCH_CANCEL_INIT
      || ARCH_CANCEL_INIT (handle)
#endif
      )
  {
    fprintf (stderr,
	     LIBGCC_S_SO " must be installed for pthread_cancel to work\n");
    abort();
  }

  __libgcc_s_resume = resume;
  libgcc_s_personality = personality;
  libgcc_s_forcedunwind = forcedunwind;
  libgcc_s_getcfa = getcfa;
  /* Make sure libgcc_s_handle is written last.  Otherwise,
     pthread_cancel_init might return early even when the pointer the
     caller is interested in is not initialized yet.  */
  atomic_write_barrier ();
  libgcc_s_handle = handle;
}

void
__libc_freeres_fn_section
__unwind_freeres (void)
{
  void *handle = libgcc_s_handle;
  if (handle != NULL)
    {
      libgcc_s_handle = NULL;
      __libc_dlclose (handle);
    }
}

#if !HAVE_ARCH_UNWIND_RESUME
void attribute_hidden
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  __libgcc_s_resume(exc);
}
#endif

_Unwind_Reason_Code attribute_hidden
__gcc_personality_v0 PERSONALITY_PROTO
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_personality PERSONALITY_ARGS;
}

_Unwind_Reason_Code attribute_hidden
_Unwind_ForcedUnwind (struct _Unwind_Exception *exc, _Unwind_Stop_Fn stop,
		      void *stop_argument)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_forcedunwind (exc, stop, stop_argument);
}

_Unwind_Word attribute_hidden
_Unwind_GetCFA (struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  return libgcc_s_getcfa (context);
}
