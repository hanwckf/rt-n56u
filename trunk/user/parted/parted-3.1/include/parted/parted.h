/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 1999-2001, 2007, 2009-2012 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PARTED_H_INCLUDED
#define PARTED_H_INCLUDED

#define PED_DEFAULT_ALIGNMENT (1024 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

#if __GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
# define __attribute(arg) __attribute__ (arg)
#else
# define __attribute(arg)
#endif

#include <parted/constraint.h>
#include <parted/device.h>
#include <parted/disk.h>
#include <parted/exception.h>
#include <parted/filesys.h>
#include <parted/natmath.h>
#include <parted/unit.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern const char *ped_get_version ()
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
  __attribute ((__const__))
#endif
;

extern void* ped_malloc (size_t size);
extern void* ped_calloc (size_t size);
extern void free (void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* PARTED_H_INCLUDED */
