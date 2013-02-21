#!/bin/sh
# RAID support on sun disk type

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

# Written by Tom "spot" Callaway <tcallawa@redhat.com>
# Derived from an example by Jim Meyering <jim@meyering.net>

. "${srcdir=.}/init.sh"; path_prepend_ ../parted
ss=$sector_size_

N=2000 # number of sectors
dev=sun-disk-file
exp="BYT;\n---:${N}s:file:$ss:$ss:sun::;\n1:0s:127s:128s"
# create an empty file as a test disk
dd if=/dev/zero of=$dev bs=${ss}c count=$N 2> /dev/null || fail=1

# label the test disk as a sun disk
parted -s $dev mklabel sun > out 2>&1 || fail=1
compare /dev/null out || fail=1

# create a single partition
parted -s $dev unit s mkpart ext2 0s 127s > out 2>&1 || fail=1
compare /dev/null out || fail=1

# print the partition data in machine readable format
parted -m -s $dev unit s p > out 2>&1 || fail=1
sed "s,^.*/$dev:,---:," out > k && mv k out

# check for expected values for the partition
printf "$exp:::;\n" > exp || fail=1
compare exp out || fail=1

# set the raid flag
parted -s $dev set 1 raid >out 2>&1 || fail=1
compare /dev/null out || fail=1

# print the partition data in machine readable format again
parted -m -s $dev unit s p > out 2>&1 || fail=1
sed "s,^.*/$dev:,---:," out > k && mv k out || fail=1

# check for expected values (including raid flag) for the partition
printf "$exp:::raid;\n" > exp || fail=1
compare exp out || fail=1

Exit $fail
