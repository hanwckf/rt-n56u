/* Copyright (C) 2002 Free Software Foundation, Inc.
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

#include <list.h>
#include <bits/libc-lock.h>

struct fork_block
{
  /* Lock to protect handling of fork handlers.  */
  __libc_lock_define (, lock);

  /* Lists of registered fork handlers.  */
  list_t prepare_list;
  list_t parent_list;
  list_t child_list;
};

extern struct fork_block __fork_block attribute_hidden;

/* Elements of the fork handler lists.  */
struct fork_handler
{
  list_t list;
  void (*handler) (void);
  void *dso_handle;
};


/* Function to call to unregister fork handlers.  */
extern void __unregister_atfork (void *dso_handle) attribute_hidden;
#define UNREGISTER_ATFORK(dso_handle) __unregister_atfork (dso_handle)


/* C library side function to register new fork handlers.  */
extern int __register_atfork (void (*__prepare) (void),
			      void (*__parent) (void),
			      void (*__child) (void),
			      void *dso_handle);

#ifndef ARCH_FORK
# define ARCH_FORK() __libc_fork()
#endif
