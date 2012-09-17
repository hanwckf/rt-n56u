/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <errno.h>
#include <fork.h>
#include <bits/libc-lock.h>

struct fork_block __fork_block =
{
  .lock = PTHREAD_MUTEX_INITIALIZER,
  .prepare_list = { &__fork_block.prepare_list, &__fork_block.prepare_list },
  .parent_list = { &__fork_block.parent_list, &__fork_block.parent_list },
  .child_list = { &__fork_block.child_list, &__fork_block.child_list }
};
