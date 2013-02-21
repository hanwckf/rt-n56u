/*
    libparted - a library for manipulating disk partitions
    Copyright (C) 2004, 2007, 2009-2012 Free Software Foundation, Inc.

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

#ifndef _JOURNAL_H
#define _JOURNAL_H

#include <parted/parted.h>
#include <parted/endian.h>
#include <parted/debug.h>

#include "hfs.h"

int
hfsj_replay_journal(PedFileSystem* fs);

int
hfsj_update_jib(PedFileSystem* fs, uint32_t block);

int
hfsj_update_jl(PedFileSystem* fs, uint32_t block);

#define HFS_16_TO_CPU(x, is_little_endian) ((is_little_endian) ? (uint16_t)PED_LE16_TO_CPU(x) : (uint16_t)PED_BE16_TO_CPU(x))
#define HFS_32_TO_CPU(x, is_little_endian) ((is_little_endian) ? (uint32_t)PED_LE32_TO_CPU(x) : (uint32_t)PED_BE32_TO_CPU(x))
#define HFS_64_TO_CPU(x, is_little_endian) ((is_little_endian) ? (uint64_t)PED_LE64_TO_CPU(x) : (uint64_t)PED_BE64_TO_CPU(x))
#define HFS_CPU_TO_16(x, is_little_endian) ((is_little_endian) ? (uint16_t)PED_CPU_TO_LE16(x) : (uint16_t)PED_CPU_TO_BE16(x))
#define HFS_CPU_TO_32(x, is_little_endian) ((is_little_endian) ? (uint32_t)PED_CPU_TO_LE32(x) : (uint32_t)PED_CPU_TO_BE32(x))
#define HFS_CPU_TO_64(x, is_little_endian) ((is_little_endian) ? (uint64_t)PED_CPU_TO_LE64(x) : (uint64_t)PED_CPU_TO_BE64(x))

#endif /* _JOURNAL_H */
