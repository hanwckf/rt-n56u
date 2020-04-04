/* Definitions for POSIX spawn interface.
   Copyright (C) 2000,2003,2004,2009,2011,2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef	_SPAWN_H
#define	_SPAWN_H	1

#include <features.h>
#include <sched.h>
#define __need_sigset_t
#include <signal.h>
#include <sys/types.h>

/* For the tiny inlines (errno/free/memset).  */
#include <errno.h>
#include <string.h>
#include <stdlib.h>


/* Data structure to contain attributes for thread creation.  */
typedef struct
{
  short int __flags;
  pid_t __pgrp;
  sigset_t __sd;
  sigset_t __ss;
  struct sched_param __sp;
  int __policy;
  int __pad[16];
} posix_spawnattr_t;


/* Data structure to contain information about the actions to be
   performed in the new process with respect to file descriptors.  */
typedef struct
{
  int __allocated;
  int __used;
  struct __spawn_action *__actions;
  int __pad[16];
} posix_spawn_file_actions_t;


/* Flags to be set in the `posix_spawnattr_t'.  */
#define POSIX_SPAWN_RESETIDS		0x01
#define POSIX_SPAWN_SETPGROUP		0x02
#define POSIX_SPAWN_SETSIGDEF		0x04
#define POSIX_SPAWN_SETSIGMASK		0x08
#define POSIX_SPAWN_SETSCHEDPARAM	0x10
#define POSIX_SPAWN_SETSCHEDULER	0x20
#ifdef __USE_GNU
# define POSIX_SPAWN_USEVFORK		0x40
#endif

__BEGIN_DECLS

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS.

   This function is a possible cancellation point and therefore not
   marked with __THROW. */
extern int posix_spawn (pid_t *__restrict __pid,
			const char *__restrict __path,
			const posix_spawn_file_actions_t *__restrict
			__file_actions,
			const posix_spawnattr_t *__restrict __attrp,
			char *const __argv[__restrict_arr],
			char *const __envp[__restrict_arr]);

/* Similar to `posix_spawn' but search for FILE in the PATH.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int posix_spawnp (pid_t *__pid, const char *__file,
			 const posix_spawn_file_actions_t *__file_actions,
			 const posix_spawnattr_t *__attrp,
			 char *const __argv[], char *const __envp[]);


/* Initialize data structure with attributes for `spawn' to default values.  */
static inline
int posix_spawnattr_init (posix_spawnattr_t *__attr)
{
  memset (__attr, 0, sizeof (*__attr));
  return 0;
}

/* Free resources associated with ATTR.  */
static inline
int posix_spawnattr_destroy (posix_spawnattr_t *__attr)
{
  return 0;
}

/* Store signal mask for signals with default handling from ATTR in
   SIGDEFAULT.  */
static inline
int posix_spawnattr_getsigdefault (const posix_spawnattr_t *
					  __restrict __attr,
					  sigset_t *__restrict __sigdefault)
{
  memcpy (__sigdefault, &__attr->__sd, sizeof (sigset_t));
  return 0;
}

/* Set signal mask for signals with default handling in ATTR to SIGDEFAULT.  */
static inline
int posix_spawnattr_setsigdefault (posix_spawnattr_t *__restrict __attr,
					  const sigset_t *__restrict
					  __sigdefault)
{
  memcpy (&__attr->__sd, __sigdefault, sizeof (sigset_t));
  return 0;
}

/* Store signal mask for the new process from ATTR in SIGMASK.  */
static inline
int posix_spawnattr_getsigmask (const posix_spawnattr_t *__restrict
				       __attr,
				       sigset_t *__restrict __sigmask)
{
  memcpy (__sigmask, &__attr->__ss, sizeof (sigset_t));
  return 0;
}

/* Set signal mask for the new process in ATTR to SIGMASK.  */
static inline
int posix_spawnattr_setsigmask (posix_spawnattr_t *__restrict __attr,
				       const sigset_t *__restrict __sigmask)
{
  memcpy (&__attr->__ss, __sigmask, sizeof (sigset_t));
  return 0;
}

/* Get flag word from the attribute structure.  */
static inline
int posix_spawnattr_getflags (const posix_spawnattr_t *__restrict
				     __attr,
				     short int *__restrict __flags)
{
  *__flags = __attr->__flags;
  return 0;
}

/* Store flags in the attribute structure.  */
static inline
int posix_spawnattr_setflags (posix_spawnattr_t *_attr,
				     short int __flags)
{
#ifdef POSIX_SPAWN_USEVFORK
# define __POSIX_SPAWN_USEVFORK POSIX_SPAWN_USEVFORK
#else
# define __POSIX_SPAWN_USEVFORK 0
#endif
#define __POSIX_SPAWN_MASK (POSIX_SPAWN_RESETIDS		\
			    | POSIX_SPAWN_SETPGROUP		\
			    | POSIX_SPAWN_SETSIGDEF		\
			    | POSIX_SPAWN_SETSIGMASK		\
			    | POSIX_SPAWN_SETSCHEDPARAM		\
			    | POSIX_SPAWN_SETSCHEDULER		\
			    | __POSIX_SPAWN_USEVFORK)

  /* Check no invalid bits are set.  */
  if (__flags & ~__POSIX_SPAWN_MASK)
    return EINVAL;

  _attr->__flags = __flags;
  return 0;
#undef __POSIX_SPAWN_USEVFORK
#undef __POSIX_SPAWN_MASK
}

/* Get process group ID from the attribute structure.  */
static inline
int posix_spawnattr_getpgroup (const posix_spawnattr_t *__restrict
				      __attr, pid_t *__restrict __pgroup)
{
  *__pgroup = __attr->__pgrp;
  return 0;
}

/* Store process group ID in the attribute structure.  */
static inline
int posix_spawnattr_setpgroup (posix_spawnattr_t *__attr,
				      pid_t __pgroup)
{
  __attr->__pgrp = __pgroup;
  return 0;
}

/* Get scheduling policy from the attribute structure.  */
static inline
int posix_spawnattr_getschedpolicy (const posix_spawnattr_t *
					   __restrict __attr,
					   int *__restrict __schedpolicy)
{
  *__schedpolicy = __attr->__policy;
  return 0;
}

/* Store scheduling policy in the attribute structure.  */
static inline
int posix_spawnattr_setschedpolicy (posix_spawnattr_t *__attr,
					   int __schedpolicy)
{
  switch (__schedpolicy) {
  case SCHED_OTHER:
  case SCHED_FIFO:
  case SCHED_RR:
    break;
  default:
    return EINVAL;
  }

  __attr->__policy = __schedpolicy;
  return 0;
}

/* Get scheduling parameters from the attribute structure.  */
static inline
int posix_spawnattr_getschedparam (const posix_spawnattr_t *
					  __restrict __attr,
					  struct sched_param *__restrict
					  __schedparam)
{
  memcpy (__schedparam, &__attr->__sp, sizeof (__attr->__sp));
  return 0;
}

/* Store scheduling parameters in the attribute structure.  */
static inline
int posix_spawnattr_setschedparam (posix_spawnattr_t *__restrict __attr,
					  const struct sched_param *
					  __restrict __schedparam)
{
  __attr->__sp = *__schedparam;
  return 0;
}

/* Initialize data structure for file attribute for `spawn' call.  */
static inline
int posix_spawn_file_actions_init (posix_spawn_file_actions_t *
					  __file_actions)
{
  memset (__file_actions, 0, sizeof (*__file_actions));
  return 0;
}

/* Free resources associated with FILE-ACTIONS.  */
static inline
int posix_spawn_file_actions_destroy (posix_spawn_file_actions_t *
					     __file_actions)
{
  free (__file_actions->__actions);
  return 0;
}

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `open' for the given file during the `spawn' call.  */
extern int posix_spawn_file_actions_addopen (posix_spawn_file_actions_t *
					     __restrict __file_actions,
					     int __fd,
					     const char *__restrict __path,
					     int __oflag, mode_t __mode)
     __THROW;

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `close' for the given file descriptor during the `spawn' call.  */
extern int posix_spawn_file_actions_addclose (posix_spawn_file_actions_t *
					      __file_actions, int __fd)
     __THROW;

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `dup2' for the given file descriptors during the `spawn' call.  */
extern int posix_spawn_file_actions_adddup2 (posix_spawn_file_actions_t *
					     __file_actions,
					     int __fd, int __newfd) __THROW;

__END_DECLS

#endif /* spawn.h */
