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

#include <semaphore.h>
#include "pthreadP.h"


/* Mount point of the shared memory filesystem.  */
struct mountpoint_info
{
  char *dir;
  size_t dirlen;
};

/* Keeping track of currently used mappings.  */
struct inuse_sem
{
  dev_t dev;
  ino_t ino;
  int refcnt;
  sem_t *sem;
  char name[0];
};


/* Variables used in multiple interfaces.  */
extern struct mountpoint_info mountpoint attribute_hidden;

extern pthread_once_t __namedsem_once attribute_hidden;

/* The search tree for existing mappings.  */
extern void *__sem_mappings attribute_hidden;

/* Lock to protect the search tree.  */
extern int __sem_mappings_lock attribute_hidden;


/* Initializer for mountpoint.  */
extern void __where_is_shmfs (void) attribute_hidden;

/* Comparison function for search in tree with existing mappings.  */
extern int __sem_search (const void *a, const void *b) attribute_hidden;
