/* Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/sem.h>
#include <stddef.h>
#include <stdlib.h> /* for NULL */

#include "ipc.h"


#ifdef L_semctl
/* Return identifier for array of NSEMS semaphores associated with
   KEY.  */
#include <stdarg.h>
/* arg for semctl system calls. */
union semun {
    int val;			/* value for SETVAL */
    struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;		/* array for GETALL & SETALL */
    struct seminfo *__buf;		/* buffer for IPC_INFO */
    void *__pad;
};


#ifdef __NR_semctl
#define __NR___semctl __NR_semctl
static __inline__ _syscall4(int, __semctl, int, semid, int, semnum, int, cmd, void *, arg)
#endif

int semctl(int semid, int semnum, int cmd, ...)
{
    union semun arg;
    va_list ap;

    /* Get the argument.  */
    va_start (ap, cmd);
    arg = va_arg (ap, union semun);
    va_end (ap);
#ifdef __NR_semctl
    return __semctl(semid, semnum, cmd | __IPC_64, arg.__pad);
#else
    return __syscall_ipc(IPCOP_semctl, semid, semnum, cmd|__IPC_64, &arg, NULL);
#endif
}
#endif

#ifdef L_semget
#ifdef __NR_semget
_syscall3(int, semget, key_t, key, int, nsems, int, semflg)

#else
/* Return identifier for array of NSEMS semaphores associated
 * with KEY.  */
int semget (key_t key, int nsems, int semflg)
{
    return __syscall_ipc(IPCOP_semget, key, nsems, semflg, NULL, 0);
}
#endif
#endif

#ifdef L_semop

#ifdef __NR_semop
_syscall3(int, semop, int, semid, struct sembuf *, sops, size_t, nsops)

#else
/* Perform user-defined atomical operation of array of semaphores.  */
int semop (int semid, struct sembuf *sops, size_t nsops)
{
    return __syscall_ipc(IPCOP_semop, semid, (int) nsops, 0, sops, NULL);
}
#endif
#endif

#ifdef L_semtimedop

#ifdef __NR_semtimedop
_syscall4(int, semtimedop, int, semid, struct sembuf *, sops, size_t, nsops, const struct timespec *, timeout)

#else

int semtimedop(int semid, struct sembuf *sops, size_t nsops,
	       const struct timespec *timeout)
{
    return __syscall_ipc(IPCOP_semtimedop, semid, nsops, 0, sops, (void *) timeout);
}
#endif
#endif
