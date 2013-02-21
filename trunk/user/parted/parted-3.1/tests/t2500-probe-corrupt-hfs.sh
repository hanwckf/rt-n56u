#!/bin/sh
# Do not misbehave when probing a corrupt HFS partition.

# Copyright (C) 2011-2012 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. "${srcdir=.}/init.sh"; path_prepend_ ../parted
ss=$sector_size_

N=3M
dev=loop-file
# create a file large enough to hold a GPT partition table
dd if=/dev/null of=$dev bs=1 seek=$N || framework_failure

parted -s "$dev" mklabel gpt mkpart p1 1MiB 2MiB > out 2>&1 || fail=1
compare /dev/null out || fail=1

parted -s "$dev" u s p || fail=1

# Poke an HFS+ signature into place
printf '\x48\x2b' | dd of=$dev seek=$((2048+2)) conv=notrunc || fail=1

# Or, if starting from a valid HFS/HFS+ file system, poke these:
# offset 18 total_blocks=0(16b)
# offset 20 vh->block_size=0(32b)

parted -s "$dev" u s p || fail=1

Exit $fail
