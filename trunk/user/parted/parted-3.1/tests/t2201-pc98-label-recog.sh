#!/bin/sh
# Recognize PC98 labeled disks

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

require_512_byte_sector_size_

ss=$sector_size_
N=8192
dev=loop-file

# create a file to simulate the underlying device
dd if=/dev/null of=$dev bs=$ss seek=$N 2> /dev/null || fail=1

# label the test disk
parted -s $dev mklabel pc98 > out 2>&1 || fail=1
compare /dev/null out || fail=1 # expect no output

parted -s $dev p | grep "^Partition Table: pc98" || fail=1

for s in "Linux 98" "GRUB/98 "; do
    printf "$s" | dd bs=1c seek=4 of=$dev conv=notrunc || fail=1
    parted -s $dev p | grep "^Partition Table: pc98" || fail=1
done

Exit $fail
