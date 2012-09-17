/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
 *
 * modified for uClibc by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>
#include <fnmatch.h>
#include <glob.h>


__ptr_t (*__glob_opendir_hook) __P ((const char *directory));
const char *(*__glob_readdir_hook) __P ((__ptr_t stream));
void (*__glob_closedir_hook) __P ((__ptr_t stream));

