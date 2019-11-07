/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <spawn.h>
#include <unistd.h>

#include "spawn_int.h"

/* Add an action to FILE-ACTIONS which tells the implementation to call
   `open' for the given file during the `spawn' call.  */
int
posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *file_actions,
				 int fd, const char *path, int oflag,
				 mode_t mode)
{
	int maxfd = sysconf(_SC_OPEN_MAX);
	struct __spawn_action *rec;

	/* Test for the validity of the file descriptor.  */
	if (fd < 0 || fd >= maxfd)
		return EBADF;

	/* Allocate more memory if needed.  */
	if (file_actions->__used == file_actions->__allocated
	    && __posix_spawn_file_actions_realloc (file_actions) != 0)
		/* This can only mean we ran out of memory.  */
		return ENOMEM;

	/* Add the new value.  */
	rec = &file_actions->__actions[file_actions->__used];
	rec->tag = spawn_do_open;
	rec->action.open_action.fd = fd;
	rec->action.open_action.path = path;
	rec->action.open_action.oflag = oflag;
	rec->action.open_action.mode = mode;

	/* Account for the new entry.  */
	++file_actions->__used;
	return 0;
}
