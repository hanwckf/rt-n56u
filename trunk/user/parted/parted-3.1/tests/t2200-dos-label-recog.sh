#!/bin/sh
# improved MSDOS partition-table recognition

# Copyright (C) 2008-2012 Free Software Foundation, Inc.

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

######################################################################
# With vestiges of a preceding FAT file system boot sector in the MBR,
# parted 1.8.8.1.29 and earlier would fail to recognize a DOS
# partition table.
######################################################################
ss=$sector_size_
N=8192
dev=loop-file

# create a file to simulate the underlying device
dd if=/dev/null of=$dev bs=$ss seek=$N 2> /dev/null || fail=1

# label the test disk
parted -s $dev mklabel msdos > out 2>&1 || fail=1
compare /dev/null out || fail=1 # expect no output

# create two partitions
parted -s $dev mkpart primary 2048s 4095s \
               mkpart primary 4096s 8191s > out 2>&1 || fail=1
compare /dev/null out || fail=1 # expect no output

# write "FAT" where it would cause trouble
printf FAT | dd bs=1c seek=82 count=3 of=$dev conv=notrunc || fail=1

# print the partition table
parted -m -s $dev unit s p > out || fail=1
tail -2 out > k && mv k out || fail=1
printf "1:2048s:4095s:2048s:::;\n2:4096s:8191s:4096s:::;\n" > exp || fail=1

compare exp out || fail=1

Exit $fail
