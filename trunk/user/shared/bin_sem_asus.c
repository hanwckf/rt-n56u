/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>

#include "bin_sem_asus.h"

/* ipc semaphore id */
int semid_bs = -1;

/* Obtain a binary semaphore's ID, allocating if necessary. */
int bin_sem_alloc(key_t key, int sem_flags)
{
	return semget (key, 1, sem_flags);
}

/* Deallocate a binary semaphore. All users must have finished their 
   use. Returns -1 on failure. */
int bin_sem_dealloc(int semid)
{
	union semun ignored_argument;

	return semctl (semid, 1, IPC_RMID, ignored_argument);
}

/* Initialize a binary semaphore with a value of 1. */
int bin_sem_init()
{
	union semun argument;
	unsigned short values[1];
	values[0] = 1;
	argument.array = values;

	semid_bs = bin_sem_alloc(KEY_ID, IPC_CREAT | IPC_EXCL | 1023);

	if (semid_bs == -1)
	{
		if (errno == EEXIST)
		{
			semid_bs = bin_sem_alloc(KEY_ID, 0);
			if (semid_bs == -1)
				return -1;
		}
		else
			return -1;
	}

	return semctl (semid_bs, 0, SETALL, argument);
}

/* Wait on a binary semaphore. Block until the semaphore value is positive, then
   decrement it by 1. */
int bin_sem_wait()
{
	struct sembuf operations[1];
	/* Use the first (and only) semaphore. */
	operations[0].sem_num = 0;
	/* Decrement by 1. */
	operations[0].sem_op = -1;
	/* Permit undo'ing. */
	operations[0].sem_flg = SEM_UNDO;

	return semop (semid_bs, operations, 1);
}

/* Post to a binary semaphore: increment its value by 1.
   This returns immediately. */
int bin_sem_post()
{
	struct sembuf operations[1];
	/* Use the first (and only) semaphore. */ 
	operations[0].sem_num = 0;
	/* Increment by 1. */ 
	operations[0].sem_op = 1;
	/* Permit undo'ing. */ 
	operations[0].sem_flg = SEM_UNDO;

	return semop (semid_bs, operations, 1);
}

int file_lock(char *tag)
{
	char fn[64];
	struct flock fl;
	int fd_lock = -1;
	pid_t ppid, fpid;

	snprintf(fn, sizeof(fn), "/var/lock/%s.lock", tag);
	if ((fd_lock = open(fn, O_CREAT | O_RDWR, 0666)) < 0)
		return -1;

	fpid = 0;
	ppid = getpid();

	if (read(fd_lock, &fpid, sizeof(fpid))) {
		if (ppid == fpid) {
			// don't close the file here as that will release all locks
			return -1;
		}
	}

	memset(&fl, 0, sizeof(fl));
	fl.l_type = F_WRLCK;
	fl.l_pid = ppid;

	if (fcntl(fd_lock, F_SETLKW, &fl) < 0) {
		close(fd_lock);
		return -1;
	}

	lseek(fd_lock, 0, SEEK_SET);
	write(fd_lock, &ppid, sizeof(ppid));

	return fd_lock;
}

void file_unlock(int fd_lock)
{
	if (fd_lock >= 0) {
		ftruncate(fd_lock, 0);
		close(fd_lock);
	}
}

