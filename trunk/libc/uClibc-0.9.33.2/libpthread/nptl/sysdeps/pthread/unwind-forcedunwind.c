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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <dlfcn.h>
#include <stdio.h>
#include <unwind.h>
#include <pthreadP.h>
#include <sysdep.h>
#include <libgcc_s.h>

#define __libc_dlopen(x)        dlopen(x, (RTLD_LOCAL | RTLD_LAZY))
#define __libc_dlsym            dlsym
#define __libc_dlclose		dlclose

static void *libgcc_s_handle;
static void (*libgcc_s_resume) (struct _Unwind_Exception *exc);
static _Unwind_Reason_Code (*libgcc_s_personality)
  (int, _Unwind_Action, _Unwind_Exception_Class, struct _Unwind_Exception *,
   struct _Unwind_Context *);
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
    printf (LIBGCC_S_SO " must be installed for pthread_cancel to work\n");
    abort();
  }

  PTR_MANGLE (resume);
  libgcc_s_resume = resume;
  PTR_MANGLE (personality);
  libgcc_s_personality = personality;
  PTR_MANGLE (forcedunwind);
  libgcc_s_forcedunwind = forcedunwind;
  PTR_MANGLE (getcfa);
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

void
_Unwind_Resume (struct _Unwind_Exception *exc)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  void (*resume) (struct _Unwind_Exception *exc) = libgcc_s_resume;
  PTR_DEMANGLE (resume);
  resume (exc);
}

_Unwind_Reason_Code
__gcc_personality_v0 (int version, _Unwind_Action actions,
		      _Unwind_Exception_Class exception_class,
                      struct _Unwind_Exception *ue_header,
                      struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  _Unwind_Reason_Code (*personality)
    (int, _Unwind_Action, _Unwind_Exception_Class, struct _Unwind_Exception *,
     struct _Unwind_Context *) = libgcc_s_personality;
  PTR_DEMANGLE (personality);
  return personality (version, actions, exception_class, ue_header, context);
}

_Unwind_Reason_Code
_Unwind_ForcedUnwind (struct _Unwind_Exception *exc, _Unwind_Stop_Fn stop,
		      void *stop_argument)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  _Unwind_Reason_Code (*forcedunwind)
    (struct _Unwind_Exception *, _Unwind_Stop_Fn, void *)
    = libgcc_s_forcedunwind;
  PTR_DEMANGLE (forcedunwind);
  return forcedunwind (exc, stop, stop_argument);
}

_Unwind_Word
_Unwind_GetCFA (struct _Unwind_Context *context)
{
  if (__builtin_expect (libgcc_s_handle == NULL, 0))
    pthread_cancel_init ();

  _Unwind_Word (*getcfa) (struct _Unwind_Context *) = libgcc_s_getcfa;
  PTR_DEMANGLE (getcfa);
  return getcfa (context);
}
