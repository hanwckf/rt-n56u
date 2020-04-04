/* Copyright (C) 2002, 2003, 2006, 2007 Free Software Foundation, Inc.
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

#include <lowlevellock.h>

/* The fork generation counter, defined in libpthread.  */
extern unsigned long int __fork_generation attribute_hidden;

/* Pointer to the fork generation counter in the thread library.  */
extern unsigned long int *__fork_generation_pointer attribute_hidden;

/* Lock to protect allocation and deallocation of fork handlers.  */
extern int __fork_lock attribute_hidden;

/* Elements of the fork handler lists.  */
struct fork_handler
{
  struct fork_handler *next;
  void (*prepare_handler) (void);
  void (*parent_handler) (void);
  void (*child_handler) (void);
  void *dso_handle;
  unsigned int refcntr;
  int need_signal;
};

/* The single linked list of all currently registered for handlers.  */
extern struct fork_handler *__fork_handlers attribute_hidden;


/* Function to call to unregister fork handlers.  */
extern void __unregister_atfork (void *dso_handle) attribute_hidden;
#define UNREGISTER_ATFORK(dso_handle) __unregister_atfork (dso_handle)


/* C library side function to register new fork handlers.  */
extern int __register_atfork (void (*__prepare) (void),
			      void (*__parent) (void),
			      void (*__child) (void),
			      void *dso_handle);
libc_hidden_proto (__register_atfork)

/* Add a new element to the fork list.  */
extern void __linkin_atfork (struct fork_handler *newp) attribute_hidden;
